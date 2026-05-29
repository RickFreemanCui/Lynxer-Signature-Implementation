for l in 128 256; do
./stat_test_gen --lambda $l --out stat_y_${l}_2p35.bin \
    --bits 34359738368 --seed "`date`" --threads $(nproc)
done


