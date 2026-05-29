for l in 128 192 256 384 512; do
./stat_test_gen --lambda $l --out stat_y_${l}_2p31.bin --bits 2147483648 --seed "`date`" --threads 8
done
