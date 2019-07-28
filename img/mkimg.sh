#! /bin/sh
QP=32
SAO=1
LEVEL=3
C=2.0
GAMMA=0.25
GAIN=0.125
TESTIMAGE=$1
if [ $# -ne 1 ];
then
	echo usage: $0 testimage.pgm
	exit
fi
TESTIMAGE=`basename $TESTIMAGE`
BASENAME=`basename $TESTIMAGE .pgm`

TASK=`basename $TESTIMAGE .pgm`"-q$QP-sao$SAO-c$C-gm$GAMMA-ga$GAIN-l$LEVEL"
MODEL="../$TASK.svm"

HM=~/c/HM-16.7/
HMOPT="-c $HM""cfg/encoder_intra_main_rext.cfg -cf 400 -f 1 -fr 1 --InternalBitDepth=8"
HMENC="$HM""bin/TAppEncoderStatic"
HMDEC="$HM""bin/TAppDecoderStatic"
AVSNR=~/c/avsnr/avsnr
ORG_DIR=~/cif_pgm/
DEC_DIR=./dec_dir

QP=32

ORG_IMG="$ORG_DIR$TESTIMAGE"
REC_IMG="$BASENAME-q$QP-sao$SAO.pgm"
MOD_IMG="$BASENAME-mod-q$QP-sao$SAO.pgm"
WIDTH=`pamfile $ORG_IMG | gawk '{print $4}'`
HEIGHT=`pamfile $ORG_IMG | gawk '{print $6}'`
tail -n +4 $ORG_IMG > input.y

$HMENC $HMOPT -q $QP -wdt $WIDTH -hgt $HEIGHT --SAO=$SAO -i input.y
$HMDEC -d 8 -b str.bin -o rec8bit.y
rawtopgm $WIDTH $HEIGHT rec8bit.y > $REC_IMG
../pfsvm_eval -S $GAIN $ORG_IMG $REC_IMG $MODEL $MOD_IMG
