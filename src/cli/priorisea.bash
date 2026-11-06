_priorisea() {
    local words=("${COMP_WORDS[@]}")
    local cword=$COMP_CWORD
    local cur=${words[cword]:-}
    local prev=${words[cword-1]:-}

    local cmd1=${words[1]:-}
    local cmd2=${words[2]:-}

    # The previous word is an option that consumes a value: complete the value
    # instead of offering more flags. For the job id --format option, offer the
    # set of accepted values.
    case "$prev" in
        --format)
            COMPREPLY=( $(compgen -W "dec hex kvs dothex words f58 f58plain" -- "$cur") )
            return 0
            ;;
        --order)
            COMPREPLY=( $(compgen -W "asc desc" -- "$cur") )
            return 0
            ;;
        --sort)
            COMPREPLY=( $(compgen -W "userid t_inactive score duration" -- "$cur") )
            return 0
            ;;
        --seanerbus|--expr|--cron|--duration|--score|--default)
            return 0
            ;;
    esac

    if [ "$cword" -eq 1 ]; then
        COMPREPLY=( $(compgen -W "daemon database formula job plugin timewin user --help" -- "$cur") )
        return 0
    fi

    case "$cmd1" in
        daemon)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "start debug update-user-period --help" -- "$cur") )
                return 0
            fi
            case "$cmd2" in
                start) COMPREPLY=( $(compgen -W "--seanerbus --background --help" -- "$cur") ) ;;
                debug) COMPREPLY=( $(compgen -W "--json --help" -- "$cur") ) ;;
                *)     COMPREPLY=( $(compgen -W "--help" -- "$cur") ) ;;
            esac
            return 0
            ;;
        database)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "create --help" -- "$cur") )
                return 0
            fi
            COMPREPLY=( $(compgen -W "--help" -- "$cur") )
            return 0
            ;;
        formula)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "get list update reset --help" -- "$cur") )
                return 0
            fi
            case "$cmd2" in
                get|list) COMPREPLY=( $(compgen -W "--json --help" -- "$cur") ) ;;
                update)   COMPREPLY=( $(compgen -W "--expr --default --help" -- "$cur") ) ;;
                reset)    COMPREPLY=( $(compgen -W "--help" -- "$cur") ) ;;
                *)        COMPREPLY=( $(compgen -W "--help" -- "$cur") ) ;;
            esac
            return 0
            ;;
        job)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "get list update --help" -- "$cur") )
                return 0
            fi
            case "$cmd2" in
                get)    COMPREPLY=( $(compgen -W "--json --detail --format --help" -- "$cur") ) ;;
                list)   COMPREPLY=( $(compgen -W "--json --detail --format --limit --order --sort --help" -- "$cur") ) ;;
                update) COMPREPLY=( $(compgen -W "--new --help" -- "$cur") ) ;;
                *)      COMPREPLY=( $(compgen -W "--help" -- "$cur") ) ;;
            esac
            return 0
            ;;
        plugin)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "sync debug --help" -- "$cur") )
                return 0
            fi
            case "$cmd2" in
                debug) COMPREPLY=( $(compgen -W "--json --help" -- "$cur") ) ;;
                sync)  COMPREPLY=( $(compgen -W "--help" -- "$cur") ) ;;
                *)     COMPREPLY=( $(compgen -W "--help" -- "$cur") ) ;;
            esac
            return 0
            ;;
        timewin)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "get list update create delete --help" -- "$cur") )
                return 0
            fi
            case "$cmd2" in
                get|list)      COMPREPLY=( $(compgen -W "--json --help" -- "$cur") ) ;;
                update|create) COMPREPLY=( $(compgen -W "--cron --duration --score --help" -- "$cur") ) ;;
                delete)        COMPREPLY=( $(compgen -W "--help" -- "$cur") ) ;;
                *)             COMPREPLY=( $(compgen -W "--help" -- "$cur") ) ;;
            esac
            return 0
            ;;
        user)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "get list update --help" -- "$cur") )
                return 0
            fi
            case "$cmd2" in
                update) COMPREPLY=( $(compgen -W "--all --help" -- "$cur") ) ;;
                get)    COMPREPLY=( $(compgen -W "--detail --json --help" -- "$cur") ) ;;
                list)   COMPREPLY=( $(compgen -W "--json --order --help" -- "$cur") ) ;;
                *)      COMPREPLY=( $(compgen -W "--help" -- "$cur") ) ;;
            esac
            return 0
            ;;
    esac
}

complete -F _priorisea priorisea
