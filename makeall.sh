
cd ..
if [ ! -d "MonitorPipe" ]; then
	git clone https://gitlab.com/setec_2020/np/MonitorPipe
fi

if [ ! -d "ws_simulator" ]; then
	git clone https://gitlab.com/setec_2020/np/ws_simulator
fi

if [ ! -d "hw_simulator" ]; then
	git clone https://gitlab.com/setec_2020/np/hw_simulator
fi

if [ ! -d "simulator" ]; then
	git clone https://gitlab.com/setec_2020/np/simulator
fi

if [ ! -d "protocol" ]; then
	git clone https://gitlab.com/setec_2020/np/protocol
fi

cd ./MonitorPipe
rm -f ./NPipe
make
cd ..

cd ./ws_simulator
rm -f ./ws_sim
make
cd ..

cd ./hw_simulator
rm -f ./hw_sim
make
cd ..

cd ./simulator
rm -f ./sim
make
cd ..

cd ./protocol
rm -f ./NP
make
cd ..

