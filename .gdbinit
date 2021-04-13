# set follow-fork-mode child
# set detach-on-fork off

# For debugging testcases
tcatch fork
commands
    printf "first fork (should be mkdirs)"
    tcatch fork
    commands
        printf "second fork (should be java)"
        tcatch fork
        commands
            printf "second fork (should be java)"
            set follow-fork-mode child
            set detach-on-fork off
            continue
        end
        continue
    end
    continue
end
