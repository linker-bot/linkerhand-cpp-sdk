#!/bin/bash

VERSION="1.1.6"

function LoadedColor_information()
{
  # 文本颜色
  BLACK='\033[0;30m'
  # 黑色
  RED='\033[0;31m'
  # 红色
  GREEN='\033[0;32m'
  # 绿色
  YELLOW='\033[0;33m'
  # 黄色
  BLUE='\033[0;34m'
  # 蓝色
  PURPLE='\033[0;35m'
  # 紫色
  CYAN='\033[0;36m'
  # 青色
  WHITE='\033[0;37m'
  # 白色

  RED_WHITE='\033[47;31m'
  # 红色
  GREEN_WHITE='\033[47;32m'
  # 绿色
  WHITE_BLACK='\033[40;37m'
  # 白色+黑色
  YELLOW_WHITE='\033[47;33m'

  # 黄色+白色
  # 背景颜色
  BLACK_B='\033[40m'
  # 黑色
  RED_B='\033[41m'
  # 红色
  GREEN_B='\033[42m'
  # 绿色
  yellow_B='\033[43m'
  # 黄色
  blue_B='\033[44m'
  # 蓝色
  purple_B='\033[45m'
  # 紫色
  Cyan_B='\033[46m'
  # 青色
  White_B='\033[47m'
  # 白色

  # 文本样式
  RESET='\033[0m'
  # 重置所有属性
  BOLD='\033[1m'
  # 粗体
  underline='\033[4m'
  # 下划线
  Blinking='\033[5m'
  # 闪烁
  Invert='\033[7m'
  # 反显
  Hidden='\033[8m'
  # 隐藏
#   echo "========================================================="
#   echo -e "${GREEN}Successfully loaded color information !${RESET}"
#   echo "========================================================="
}

CLOSE_RESERVE="exce bash" # exce bash

RUN() {
	local launch=$1
	gnome-terminal -- bash -c "source devel/setup.sh; $launch; $CLOSE_RESERVE"
	sleep 3
}


RUN1() {
	local launch=$1
	local command package launch_file

    IFS=' ' read -r command package launch_file <<< "$launch"
	
	gnome-terminal -- bash -c "source devel/setup.sh; $launch; $CLOSE_RESERVE"  > /dev/null 2>&1
	
	echo "Node: $package"
    while ! rosnode list | grep -q $package; do
      echo -e "  starting..."
      sleep 1
    done
    echo -e "  Startup completed"
}


function build_sdk(){
    mkdir build; cd build; cmake ..; make; cd ..
}

function install_sdk(){
    cd build; sudo make install
}

function uninstall_sdk(){
    sudo rm -rf /usr/local/linker_hand_cpp_sdk/
}

function run_example(){
    cd build; ./linker_hand_example
}

#------------------------------------------------ Select Menu ------------------------------------------

function select_menu(){
  cd $current_dir
  echo -e "${GREEN}Please enter options: ${RESET}"
  read -p "" select_num
  case $select_num in
      1)
        echo "Build SDK"
        build_sdk
        ;;
      2)
      	echo "Install SDK"
        build_sdk
        install_sdk
        ;;
      3)
        echo "Uninstall SDK"
        uninstall_sdk
        ;;
      6)
        echo "Execution Example"
        run_example
        ;;
      0)
        echo "Exit"
        exit
        ;;
      *)
        echo -e "${RED}Input error, please re-enter！${RESET}"
        sleep 1
        ;;
  esac
}

#------------------------------------------------ Menu ------------------------------------------
function show_Info(){
  echo -e "${YELLOW}"
  echo "================================================"
  echo -e "${YELLOW} LinkerHand CPP-SDK  Version：${VERSION} ${RESET}"
  echo -e "${YELLOW}================================================${RESET}"

  echo -e "${GREEN}"
  echo "RUN Choose Task:"
  echo -e "${YELLOW}————————————————————————————————————————————————${RESET}"
  echo -e "${BLUE}[1]: Build SDK${RESET}"
  echo -e "${YELLOW}————————————————————————————————————————————————${RESET}"
  echo -e "${BLUE}[2]: Install SDK${RESET}"
  echo -e "${YELLOW}————————————————————————————————————————————————${RESET}"
  echo -e "${BLUE}[3]: Uninstall SDK${RESET}"
  echo -e "${YELLOW}————————————————————————————————————————————————${RESET}"
  echo -e "${RED}[0]: Exit${RESET}"
  echo -e "${YELLOW}————————————————————————————————————————————————${RESET}"
  # sudo make DESTDIR=/home/lst/Desktop/install install
}

#------------------------------------------------ Init ------------------------------------------

function Init()
{
    LoadedColor_information
  	current_dir=$(pwd)
    sleep 1
}

#------------------------------------------------ Main ------------------------------------------

Init
while true
do
    show_Info
    select_menu
done
