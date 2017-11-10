#!/bin/bash
# Bash Curecoin Installer

PS3='Please enter your choice: '
options=("Install Full Curecoin Wallet" "Install Curecoin Daemon" "Install dependencies" "Install all" "Exit")
select opt in "${options[@]}"
do
    case $opt in
        "Install Full Curecoin Wallet")
            qmake && make && echo Done installing && break
            ;;
        "Install Curecoin Daemon")
            cd src && make -f makefile.unix && echo Done installing && break
            ;;
        "Install dependencies")
            sudo apt-get install qt5-default qt5-qmake qtbase5-dev-tools qttools5-dev-tools libboost-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev libssl-dev libminiupnpc-dev libdb5.3++-dev dh-make build-essential && echo Done installing dependencies. Relaunch to install Curecoin && break
            ;;
	"Install all")
            sudo apt-get install qt5-default qt5-qmake qtbase5-dev-tools qttools5-dev-tools libboost-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev libssl-dev libminiupnpc-dev libdb5.3++-dev dh-make build-essential && qmake && make && cd src && make -f makefile.unix && echo Done installing && break
            ;;
        "Exit")
            echo "Visit curecoin.net/forum for support" && break
            ;;
        *) echo invalid option;;
    esac
done
