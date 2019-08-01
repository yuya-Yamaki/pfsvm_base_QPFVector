QPを特徴慮に含めた学習を行うプログラム，学習時はQPの異なるものを画像毎に用意している．そのため，学習枚数は4倍になっている．評価時も同様にQPを特徴量に含め，予測させる．CIF対応
loo.sh : cif→cif
looHD.sh : cif→HD
LearnEvalHD_pfsvm.sh : HD→HD

default
cfg : encoder_intra_main_rext.cfg
sample_ratio : 0.1
ga : 0.125
gm : 0.25
c : 2.0
