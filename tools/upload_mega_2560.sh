port="$1"
dir="$2"
board='arduino:avr:mega'

arduino-cli compile $2 -b $board || exit 1
arduino-cli upload $2 -p $port -b $board
