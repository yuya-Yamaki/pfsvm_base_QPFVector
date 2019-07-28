#! /bin/sh
#このプログラムはお試し用でそもそも再生画像を入力として用いていない

#定義式みたいな部分
#,DIR=/rda2/DATABASE/TMW,
#$TRAINING=camera,
#$QUALITY=75,
DIR="/rda2/DATABASE/TMW"
TRAINING=camera
QUALITY=75
MODEL="model-$TRAINING-q$QUALITY.svm"
LOG=`basename $MODEL .svm`".log"

#test.shを実際に実行してみると以下の二文が最初に表示される．
echo "Running at "`uname -a` | tee $LOG
#rda2/DATABASE/TMW/camera.pgmを機械学習用画像として入力する
echo "Training image is $TRAINING.pgm" | tee -a $LOG
pnmtojpeg -quality=$QUALITY "$DIR/$TRAINING.pgm" > tmp.jpg
jpegtopnm tmp.jpg > decoded.pgm

#試しに一枚の画像で機械学習させて輝度補償をさせている
#おそらくpfsvm_train.oとpfsvm_eval.oを実行させている
(time ./pfsvm_train "$DIR/$TRAINING.pgm" decoded.pgm $MODEL) 2>&1 | tee -a $LOG
(time ./pfsvm_eval "$DIR/$TRAINING.pgm" decoded.pgm $MODEL modified.pgm) 2>&1 | tee -a $LOG

#for loop /rda2/DATABASE/TMW/に入っている画像をtestimageとして用いている．
for IMG in `ls $DIR/*.pgm`
do
	if [ `basename $IMG .pgm` != $TRAINING ];
	then
		echo "-----------------------------------" | tee -a $LOG
		echo "Evaluation for "`basename $IMG` | tee -a $LOG
		pnmtojpeg -quality=$QUALITY $IMG > tmp.jpg
		jpegtopnm tmp.jpg > decoded.pgm
		(time ./pfsvm_eval $IMG decoded.pgm $MODEL modified.pgm) 2>&1 | tee -a $LOG
	fi
done
