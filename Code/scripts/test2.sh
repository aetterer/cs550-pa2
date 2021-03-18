#!/bin/bash
ip=$(ifconfig | grep 192.168 | awk '{print $2}')
results=../Out/test2_output/test2_results.csv
num_rq=1000

echo -n "" > $results
echo -n "" > rq.tmp

# Populate request file.
for ((i=0; i<$num_rq; i++)); do
    echo "list" >> rq.tmp
done
echo "quit" >> rq.tmp

# Start the indexing server.
echo "starting indexing server"
./bin/index_server configs/indexserver.cfg &> /dev/null &
#ispid=$1


for num_nodes in 2 3 4 5 6 7 8; do
    # Generate fake files in each watched directory.
    for i in $(seq 1 $num_nodes); do
        path=$(sed -n 2p configs/peernode${i}.cfg)
        for j in 1..10; do
            fn=tmp.$(pwgen -N1)
            touch ${path}${fn}
        done
    done

    pidlist=()

    # Star the peer nodes and collect their PIDs.
    echo "starting $num_nodes peer nodes"
    for i in $(seq 1 $num_nodes); do
        #fuser -k $(sed -n 1p configs/peernode${i}.cfg)/tcp > /dev/null
        ./bin/peer_node configs/peernode${i}.cfg $ip 20769 < rq.tmp &> /dev/null &
        pidlist+=($!)
    done

    # Wait for all nodes to finish.
    echo "waiting for peer nodes to finish"
    for p in "${pidlist[@]}"; do
        wait $p
    done

    # Calculate the average response time and store in a csv.
    echo "calculating average time"
    for i in $(seq 1 $num_nodes); do
        awk '{print $6}' logs/node${i}.log >> res.tmp
        cp logs/node${i}.log ../Out/test2_output/node${i}_test2_${num_nodes}nodes.log
    done

    # Remove fake files.
    for i in $(seq 1 $num_nodes); do
        path=$(sed -n 2p configs/peernode${i}.cfg)
        rm ${path}*
    done
    

    sum=0
    total=0
    while read line; do
        sum=$(echo "scale=6;$sum + $line" | bc -l)
        total=$(echo "scale=6;$total + 1" | bc -l)
    done < res.tmp
    avg=$(echo "scale=6;$sum / $total" | bc -l)
    echo "$num_nodes,0$avg" >> $results

    rm res.tmp
done

rm rq.tmp

#kill $ispid

echo "all done!"
