#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include "svm.h"
#include "pfsvm.h"
struct svm_parameter param;
struct svm_problem prob;
struct svm_model *model;
struct svm_node *x_space;
#define LEAVE_ONE_OUT
#define RND_SEED 12345L
#ifdef LEAVE_ONE_OUT
#  define MAX_IMAGE 256
#  define SAMPLE_RATIO 0.1
#else
#  define MAX_IMAGE 1
#  define SAMPLE_RATIO 1.01
#endif

/*
    struct svm_problem describes the problem:

	struct svm_problem
	{
		int l;
		double *y;
		struct svm_node **x;
	};

    where `l' is the number of training data, and `y' is an array containing
    their target values. (integers in classification, real numbers in
    regression) `x' is an array of pointers, each of which points to a sparse
    representation (array of svm_node) of one training vector.

    For example, if we have the following training data:

    LABEL    ATTR1    ATTR2    ATTR3    ATTR4    ATTR5
    -----    -----    -----    -----    -----    -----
      1        0        0.1      0.2      0        0
      2        0        0.1      0.3     -1.2      0
      1        0.4      0        0        0        0
      2        0        0.1      0        1.4      0.5
      3       -0.1     -0.2      0.1      1.1      0.1

    then the components of svm_problem are:

    l = 5

    y -> 1 2 1 2 3

    x -> [ ] -> (2,0.1) (3,0.2) (-1,?)
         [ ] -> (2,0.1) (3,0.3) (4,-1.2) (-1,?)
         [ ] -> (1,0.4) (-1,?)
         [ ] -> (2,0.1) (4,1.4) (5,0.5) (-1,?)
         [ ] -> (1,-0.1) (2,-0.2) (3,0.1) (4,1.1) (5,0.1) (-1,?)

    where (index,value) is stored in the structure `svm_node':

	struct svm_node
	{
		int index;
		double value;
	};

    index = -1 indicates the end of one vector. Note that indices must
    be in ASCENDING order.
 */

/*leave one outという方式でsvmを行っているプログラム
pfsvm_train.cと全く同じコード
統計学において標本データを分割し，その一部をまず解析して，残る部分でその解析のテストを行い，解析自身の妥当性の検証・確認に当てる手法*/

int set_images(char *org_dir, char *dec_dir, IMAGE **oimg_list, IMAGE **dimg_list, int *QP_param)
{
#ifdef LEAVE_ONE_OUT
    FILE *fp;
    DIR *dir;
    struct dirent *dp;
    char org_img[256], dec_img[256];
    int num_img = 0;
	int q;
	int QP[4] = {37,32,27,22};
	char filename[256][100];
	int i = 0;

    if ((dir = opendir(org_dir)) == NULL) {
	fprintf(stderr, "Can't open directory '%s'\n", org_dir);
	exit(1);
    }
    while ((dp = readdir(dir)) != NULL) {
		if (strncmp(dp->d_name + strlen(dp->d_name) - 4, ".pgm", 4) != 0) continue;
			strncpy(org_img, org_dir, 255);
		if (org_img[strlen(org_img) - 1] != '/') {
		    strcat(org_img, "/");
		}
		//org_img = /rda1/users/yamaki/cif_pgm/
		strcat(org_img, dp->d_name);
		//org_img = /rda1/users/yamaki/cif_pgm/akiyo.pgm
		for (q = 0; q < 4; q++)
		{
			strncpy(dec_img, dec_dir, 255);
			if (dec_img[strlen(dec_img) - 1] != '/')
			{
				strcat(dec_img, "/");
			}
			strcat(dec_img, dp->d_name);
			//./dec_dir/akiyo-dec.pgm
			strncpy(filename[i], dec_img, (strlen(dec_img)-4));
			sprintf(dec_img, "%s-%d.pgm", filename[i], QP[q]);
			strcpy(dec_img + strlen(dec_img) - 4, "-dec.pgm");
			//./dec_dir/carphone-dec.pgm
			if ((fp = fopen(dec_img, "r")) == NULL){//評価画像の画像で学習しない．dec_dirにないため評価画像の復号画像はNULLを吐く
				i++;
				continue;
			}
			fclose(fp);
			QP_param[num_img] = QP[q];
			printf("%s %s\n", org_img, dec_img);
			oimg_list[num_img] = read_pgm(org_img);
			dimg_list[num_img] = read_pgm(dec_img);
			num_img++;
			i++;
		}//for
    }//while
    return (num_img);
#else
    oimg_list[0] = read_pgm(org_dir);
    dimg_list[0] = read_pgm(dec_dir);
    return (1);
#endif
}

int main(int argc, char **argv)
{
    IMAGE *oimg_list[MAX_IMAGE], *dimg_list[MAX_IMAGE], *org, *dec;
    int cls[MAX_CLASS];
    int i, j, k, m, n, label, img;
    int num_img, num_class = 3;
    size_t elements;
    double th_list[MAX_CLASS/2], fvector[NUM_FEATURES], sig_gain = 1.0;
    const char *error_msg;
    static double svm_c = 1.0, svm_gamma = 1.0 / NUM_FEATURES;
    static char *org_dir = NULL, *dec_dir = NULL, *modelfile = NULL;
	int QP_param[MAX_IMAGE];

    cpu_time();
    setbuf(stdout, 0);
    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {
	    case 'L':
		num_class = atoi(argv[++i]);
		if (num_class < 3 || num_class > MAX_CLASS || (num_class % 2) == 0) {
		    fprintf(stderr, "# of classes is wrong!\n");
		    exit (1);
		}
		break;
	    case 'C':
		svm_c = atof(argv[++i]);
		break;
	    case 'G':
		svm_gamma = atof(argv[++i]);
		break;
	    case 'S':
		sig_gain = atof(argv[++i]);
		break;
	    default:
		fprintf(stderr, "Unknown option: %s!\n", argv[i]);
		exit (1);
	    }
	} else {
	    if (org_dir == NULL) {
		org_dir = argv[i];
	    } else if (dec_dir == NULL) {
		dec_dir = argv[i];
	    } else {
		modelfile = argv[i];
	    }
	}
    }
	if (modelfile == NULL) {
#ifdef LEAVE_ONE_OUT
	printf("Usage: %s [options] original_dir decoded_dir model.svm\n",
#else
	printf("Usage: %s [options] original.pgm decoded.pgm model.svm\n",
#endif
	       argv[0]);
	printf("    -L num  The number of classes [%d]\n", num_class);
	printf("    -C num  Penalty parameter for SVM [%f]\n", svm_c);
	printf("    -G num  Gamma parameter for SVM [%f]\n", svm_gamma);
	printf("    -S num  Gain factor for sigmoid function [%f]\n", sig_gain);
	exit(0);
    }
    num_img = set_images(org_dir, dec_dir, oimg_list, dimg_list, QP_param);//ここで画像リストをセットしている
    set_thresholds(oimg_list, dimg_list, num_img, num_class, th_list);
    printf("Number of classes = %d\n", num_class);
    printf("Number of training images = %d\n", num_img);
    printf("Thresholds = {%.1f", th_list[0]);
    for (k = 1; k < num_class / 2; k++) {
	printf(", %.1f", th_list[k]);
    }
    printf("}\n");
    printf("Gain factor = %f\n", sig_gain);
	printf("SVM(gamma, C) = (%f,%f)\n", svm_gamma, svm_c);

    elements = 0;
    prob.l = 0;
    srand48(RND_SEED);  //drand48()のための初期化
    for (img = 0; img < num_img; img++) {
	org = oimg_list[img];
	dec = dimg_list[img];
	for (i = 0; i < dec->height; i++) {
	    for (j = 0; j < dec->width; j++) {
		if (drand48() < SAMPLE_RATIO) { //10%の確率で真(SAMPLW_RATIO = 0.1)
		    elements += get_fvector(dec, i, j, sig_gain, fvector);
			elements += 1;//QP特徴量の追加
		    prob.l++;//あくまで学習データ数は同じなので変更なし（特徴量が1次元増えただけにすぎない）
		}
	    }
	}
    }
    printf("Number of samples = %d (%d)\n", prob.l, (int)elements);

    /* Setting for LIBSVM */
    param.svm_type = C_SVC;
    param.kernel_type = RBF;
    param.degree = 3; /* for poly */
    param.gamma = svm_gamma;  /* for poly/rbf/sigmoid */
    param.coef0 = 0;  /* for poly/sigmoid */

  /* these are for training only */
    param.nu = 0.5; /* for NU_SVC, ONE_CLASS, and NU_SVR */
    param.cache_size = 100; /* in MB */
    param.C = svm_c;  /* for C_SVC, EPSILON_SVR and NU_SVR */
    param.eps = 1e-3; /* stopping criteria */
    param.p = 0.1;  /* for EPSILON_SVR */
    param.shrinking = 0; // Changed /* use the shrinking heuristics */
    param.probability = 0;  /* do probability estimates */
    param.nr_weight = 0;  /* for C_SVC */
    param.weight_label = NULL;  /* for C_SVC */
    param.weight = NULL;  /* for C_SVC */
    elements += prob.l;
    prob.y = Malloc(double, prob.l);
    prob.x = Malloc(struct svm_node *, prob.l);
    x_space = Malloc(struct svm_node, elements);
    for (k = 0; k < num_class; k++) cls[k] = 0;
    m = n = 0;
    srand48(RND_SEED); //drand48のためにsrand48関数で初期化
    for (img = 0; img < num_img; img++) {
	org = oimg_list[img];
	dec = dimg_list[img];
	for (i = 0; i < dec->height; i++) {
	    for (j = 0; j < dec->width; j++) {
		if (drand48() < SAMPLE_RATIO) {
		    label = get_label(org, dec, i, j, num_class, th_list);
		    cls[label]++;
		    prob.y[m] = label;
		    prob.x[m] = &x_space[n];//prob.x is defined struct svm_node **x; in struct svm_problem→これでx_space[n]に特徴ベクトルを入れるとprob.xから参照できる
		    get_fvector(dec, i, j, sig_gain, fvector);
		    for (k = 0; k < NUM_FEATURES; k++) {
			if (fvector[k] != 0.0) {
			    x_space[n].index = k + 1;
			    x_space[n].value = fvector[k];
			    n++;
			}
			//--------------------------NUM_FEATURES=12で固定，k=12の時は，13次元ベクトルにしたいため，QPを追加する 0726-------------------//
		    }
			if (k == 12){
				x_space[n].index = k + 1;
				//QP_param[num_img] = 2.0 / (1 + exp(-(double)QP_param[num_img] * gain)) - 1.0;
				x_space[n].value = QP_param[num_img];
				n++;
			}
		    x_space[n++].index = -1;
		    m++;
		}
	    }
	}
	}
    for (k = 0; k < num_class; k++) {
	printf("CLASS[%d] = %d\n", k, cls[k]);
    }
    error_msg = svm_check_parameter(&prob,&param);
    if (error_msg) {
	fprintf(stderr,"ERROR: %s\n",error_msg);
	exit(1);
    }
    model = svm_train(&prob, &param);
    if (svm_save_model(modelfile, model)) {
	fprintf(stderr, "Can't save model to file %s\n", modelfile);
	exit(1);
    }
    svm_free_and_destroy_model(&model);
    svm_destroy_param(&param);
    free(prob.y);
    free(prob.x);
    free(x_space);
    printf("cpu time: %.2f sec.\n", cpu_time());
    return (0);
}
