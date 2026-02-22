printf "\nInstalled files:\n"
if [ -f install_manifest.txt ]; then
    LS=$(cat install_manifest.txt)
else
    if [ -f build/install_manifest.txt ]; then
        if [ -f manifest.txt ]; then
            if [ build/install_manifest.txt -nt manifest.txt ]; then
                LS=$(cat build/install_manifest.txt)
            else
                LS=$(cat manifest.txt)
            fi
        else
            LS=$(cat build/install_manifest.txt)
        fi
    else
        if [ -f manifest.txt ]; then
            LS=$(cat manifest.txt)
        else
            echo no manifest.txt or install_manifest.txt
        fi
    fi
fi
if which lsd >/dev/null 2>&1; then
    lsd -Ul --icon-theme unicode $LS
else
    ls -Ul --color=always $LS
fi
