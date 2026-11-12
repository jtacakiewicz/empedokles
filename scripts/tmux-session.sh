#!/usr/bin/env bash

SESSION_NAME="emp-dev"

tmux new-session -d -s $SESSION_NAME -n nvim "nix develop . -c zsh" && tmux send-keys -t $SESSION_NAME:1 'nvim' Enter
tmux new-window -t $SESSION_NAME:2 -n build "nix develop . -c zsh"
tmux new-window -t $SESSION_NAME:3 -n lazygit 'nix develop . -c lazygit'
tmux select-window -t $SESSION_NAME:1
tmux attach -t $SESSION_NAME
