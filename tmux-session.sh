#!/usr/bin/env bash

SESSION_NAME="emp-dev"

# Start a new tmux session with the first window running nvim
tmux new-session -d -s $SESSION_NAME -n nvim "nix-shell . --run nvim"


# Create second window (idle)
tmux new-window -t $SESSION_NAME:2 -n build "nix-shell . --run zsh"

# Create third window running lazygit
tmux new-window -t $SESSION_NAME:3 -n lazygit 'lazygit'

# Attach to session, starting in the first window
tmux select-window -t $SESSION_NAME:1
tmux attach -t $SESSION_NAME
