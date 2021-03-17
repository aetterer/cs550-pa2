#!/bin/bash
ip=$(ifconfig | grep 192.168 | awk '{print $2}')
results=../Out/test3_output/test3_results.csv
num_files=10
num_requests=10
num_nodes=4

echo -n "" > $results
echo -n "" > rq.tmp

# Start the indexing server.
echo "starting indexing server"
./bin/index_server configs/indexserver.cfg &> /dev/null &
#ispid=$1


for file_size in 512; do
    filelist=()

    # Generate fake files in each watched directory.
    echo "generating files of size $file_size B"
    for i in $(seq 1 $num_files); do
        fn=tmp.$(pwgen -N1)
        filelist+=($fn)
        for j in $(seq 1 $num_nodes); do
            path=$(sed -n 2p configs/peernode${j}.cfg)
            ./scripts/generate_file.sh ${path}${fn} $file_size
        done
    done

    pidlist=()
#
#    # Star the peer nodes and collect their PIDs.
#    echo "starting $num_nodes peer nodes"
    
    echo "generating the request file"
    for i in $(seq 1 $num_requests); do
        x=$(($num_files - 1))
        r=$(shuf -i 0-$x -n 1)
        echo "download ${filelist[$r]}" >> rq.tmp
    done
    echo "quit" >> rq.tmp

    echo "deploying the peer nodes"
    for i in $(seq 1 $num_nodes); do
        ./bin/peer_node configs/peernode${i}.cfg $ip 20769 < rq.tmp &> /dev/null &
        pidlist+=($!)
    done
#

#    # Wait for all nodes to finish.
    echo "waiting for peer nodes to finish"
    for p in "${pidlist[@]}"; do
        wait $p
    done

    # Remove files.
    rm rq.tmp
    echo "removing files"
    for i in $(seq 1 $num_nodes); do
        path=$(sed -n 2p configs/peernode${i}.cfg)
        rm ${path}*
    done
#
#    # Calculate the average response time and store in a csv.
    echo "calculating average time"
    for i in $(seq 1 $num_nodes); do
        awk '{print $6}' logs/node${i}.log >> res.tmp
        cp logs/node${i}.log ../Out/test2_output/node${i}_test2_${num_nodes}nodes.log
    done 

    sum=0
    total=0
    while read line; do
        sum=$(echo "scale=6;$sum + $line" | bc -l)
        total=$(echo "scale=6;$total + 1" | bc -l)
    done < res.tmp
    avg=$(echo "scale=6;$sum / $total" | bc -l)
    echo "$file_size,0$avg" >> $results

    rm res.tmp
done

#kill $ispid

echo "all done!"
