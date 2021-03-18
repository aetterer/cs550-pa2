index_cfg=configs/indexserver.cfg
node1_cfg=configs/peernode1.cfg
node2_cfg=configs/peernode2.cfg
node3_cfg=configs/peernode3.cfg
node4_cfg=configs/peernode4.cfg

ip=192.168.56.3
index_port=$(sed -n 1p $index_cfg)
node1_port=$(sed -n 1p $node1_cfg)
node2_port=$(sed -n 1p $node2_cfg)
node3_port=$(sed -n 1p $node3_cfg)
node4_port=$(sed -n 1p $node4_cfg)

node1_dir=$(sed -n 2p $node1_cfg)
node2_dir=$(sed -n 2p $node2_cfg)
node3_dir=$(sed -n 2p $node3_cfg)
node4_dir=$(sed -n 2p $node4_cfg)

results=../Out/test3_output/test3_results.csv
echo -n "" > $results

num_files=5
#file_size=2048

#./bin/index_server configs/indexserver &
./scripts/deploy_index.sh &

sleep 1

for file_size in 128 512 2048 8192; do
    for i in $(seq 1 $num_files); do
        ./scripts/generate_file.sh ${node1_dir}file${i} $file_size
    done
    cp $node1_dir* $node2_dir
    cp $node1_dir* $node3_dir
    cp $node1_dir* $node4_dir

    pidlist=()

    echo "Deploying nodes"
    printf 'download file1\nquit\n' | ./bin/peer_node $node1_cfg $ip $index_port &
    pidlist+=($!)
    printf 'download file1\nquit\n' | ./bin/peer_node $node2_cfg $ip $index_port &
    pidlist+=($!)

    echo "Waiting for nodes to finish"
    for p in "${pidlist[@]}"; do
        wait $p
    done

    rm $node1_dir*
    rm $node2_dir*
    rm $node3_dir*
    rm $node4_dir*

    for i in $(seq 1 2); do
        awk '{print $6}' logs/node${i}.log >> res.tmp
        cp logs/node${i}.log ../Out/test3_output/node${i}_test3_${num_nodes}nodes.log
    done

    sum=0
    total=0
    while read line; do
        sum=$(echo "scale=6;$sum + $line" | bc -l)
        total=$(echo "scale=6;$total + 1" | bc -l)
    done < res.tmp
    echo "     $sum / $total"
    avg=$(echo "scale=6;$sum / $total" | bc -l)
    echo "$file_size,0$avg" >> $results
done

echo "Done"


