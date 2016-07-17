s/([[:xdigit:]]+)[ \t]+([[:alpha:]])[ \t]([[:graph:]]+)/-Wl,--defsym=\3=0x\1/
