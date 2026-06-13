cd ~/menuapp/man
for file in $(lf -d1 -tf); do
    echo $file
done
