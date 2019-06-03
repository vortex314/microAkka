
cd $HOME/workspace/
git clone https://github.com/vortex314/Common.git
git clone https://github.com/vortex314/microAkka.git
git clone https://github.com/vortex314/stm32prog.git
git clone https://github.com/vortex314/serial2mqtt.git
for dir in ./*/     # list directories in the form "/tmp/dirname/"
do
    dir=${dir%*/}      # remove the trailing "/"
    echo "=====================================> " ${dir##*/}    # print everything after the final "/"
	cd $dir
	git pull --recurse-submodules
	cd ..
done
cd Common
make -f Common.mk
cd ../paho.mqtt.c
make
../serial2mqtt/makePaho.sh
cd ../microAkka
make -f microAkka.mk
