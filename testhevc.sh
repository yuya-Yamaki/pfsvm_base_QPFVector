#! /bin/sh
TRAINING=carphone
QP=32
SAO=0
LEVEL=3
C=0.5
GAMMA=0.5
GAIN=0.125
TASK="train-"`basename $TRAINING .pgm`"-q$QP-c$C-gm$GAMMA-ga$GAIN-l$LEVEL"
MODEL="$TASK.svm"
LOG="$TASK.log"
HM=~/HM-16.9/
HMOPT="-c $HM""cfg/encoder_intra_main_rext.cfg -cf 400 -f 1 -fr 1 --InternalBitDepth=8"
HMENC="$HM""bin/TAppEncoderStatic"
HMDEC="$HM""bin/TAppDecoderStatic"
DIR=~/cif_pgm/
IMG=$DIR$TRAINING.pgm
echo "Running at "`uname -a` | tee $LOG
echo "Training image is $TRAINING.pgm" | tee -a $LOG
WIDTH=`pamfile $IMG | gawk '{print $4}'`
HEIGHT=`pamfile $IMG | gawk '{print $6}'`
tail -n +4 $IMG > input.y
$HMENC $HMOPT -q $QP -wdt $WIDTH -hgt $HEIGHT --SAO=$SAO -i input.y
SIZE=`ls -l str.bin | gawk '{print $5}'`
$HMDEC -d 8 -b str.bin -o rec8bit.y
rawtopgm $WIDTH $HEIGHT rec8bit.y > decoded.pgm
SNR=`pnmpsnr $IMG decoded.pgm 2>&1 | gawk '{print $7}'`
printf "QP:SIZE:SNR %3d%10d%10.2f\n" $QP $SIZE $SNR | tee -a $LOG
./pfsvm_train -C $C -G $GAMMA -N $LEVEL -S $GAIN $IMG decoded.pgm $MODEL | tee -a $LOG

for IMG in `ls $DIR*.pgm`
do
	echo "-----------------------------------" | tee -a $LOG
	echo "Evaluation for "`basename $IMG` | tee -a $LOG
	tail -n +4 $IMG > input.y
	$HMENC $HMOPT -q $QP -wdt $WIDTH -hgt $HEIGHT --SAO=$SAO -i input.y
	SIZE=`ls -l str.bin | gawk '{print $5}'`
	$HMDEC -d 8 -b str.bin -o rec8bit.y
	rawtopgm $WIDTH $HEIGHT rec8bit.y > decoded.pgm
	SNR=`pnmpsnr $IMG decoded.pgm 2>&1 | gawk '{print $7}'`
	printf "QP:SIZE:SNR %3d%10d%10.2f\n" $QP $SIZE $SNR | tee -a $LOG
	./pfsvm_eval -S $GAIN $IMG decoded.pgm $MODEL modified.pgm | tee -a $LOG
done
