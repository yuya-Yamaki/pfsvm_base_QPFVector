QPを特徴慮に含めた学習を行うプログラム，学習時は4つのQP値の中からランダムに1つの値を画像に割り当てる．そのためこれまでと学習量は変わらない．また，特徴量に対しては他の特徴と同様にsigmoid関数を施しrangeをそろえている．

loo.sh : cif→cif
looHD.sh : cif→HD
LearnEvalHD_pfsvm.sh : HD→HD

default
cfg : encoder_intra_main_rext.cfg
sample_ratio : 0.01
ga : 0.125
gm : 0.25
c : 2.0
