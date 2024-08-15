#!/bin/bash
SESSION="AISVP"
RUN_QEMU="./run_qemu.sh"
RUN_systemC="./demo/test_mmio/main"

SESEXS=$(tmux ls | grep $SESSION)

if [[ $1 = "on" || $1 = "" ]]
then
    if [ "$SESEXS" = "" ]
    then
        tmux new-session -d -s $SESSION
        tmux rename-window -t $SESSION:0 'QEMU'
        tmux split-window -h -p 50 -t $SESSION:QEMU

        tmux select-pane -L
        tmux send-keys -t $SESSION:QEMU "bash" C-m
        tmux send-keys -t $SESSION:QEMU "cd ./VPqemu" C-m
        tmux send-keys -t $SESSION:QEMU $RUN_QEMU C-m

        tmux select-pane -R
        tmux send-keys -t $SESSION:QEMU "bash" C-m
        tmux send-keys -t $SESSION:QEMU "cd ./VPsystemC" C-m
        tmux send-keys -t $SESSION:QEMU $RUN_systemC C-m

        tmux select-pane -L
    fi
    tmux attach -t $SESSION:QEMU
fi
if [ $1 = "off" ]
then
    if [ "$SESEXS" != "" ]
    then
        tmux select-pane -L
        tmux select-pane -L
        tmux send-keys -t $SESSION:QEMU "exit" C-m
        tmux send-keys -t $SESSION:QEMU "exit" C-m

        tmux select-pane -R
        tmux send-keys -t $SESSION:QEMU "exit" C-m
        tmux send-keys -t $SESSION:QEMU "exit" C-m
    fi
fi