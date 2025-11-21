#!/bin/bash
echo "How lucky you are? This is Form 1."
name=$(enterstr "Enter your name: ")
echo "¡ola, $name! ¡Bienvenidos al primer curso!"
cmd_key=$(enterchr "Pulse cualquier tecla para continuar: ")
echo "$cmd_key"
echo
