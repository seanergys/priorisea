# SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
#
# SPDX-License-Identifier: MIT

_priorisea() {
    local words=("${COMP_WORDS[@]}")
    local cword=$COMP_CWORD
    local cur=${words[cword]:-}
    local prev=${words[cword-1]:-}

    local cmd1=${words[1]:-}
    local cmd2=${words[2]:-}

    # Options that consume a value: don't offer further flags right after them.
    case "$prev" in
        --seanerbus|--expr|--default|--cron|--duration|--score)
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
                COMPREPLY=( $(compgen -W "start --help" -- "$cur") )
                return 0
            fi
            COMPREPLY=( $(compgen -W "--seanerbus --background --help" -- "$cur") )
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
                get|list)  COMPREPLY=( $(compgen -W "--json --help" -- "$cur") ) ;;
                update)    COMPREPLY=( $(compgen -W "--expr --default --help" -- "$cur") ) ;;
                *)         COMPREPLY=( $(compgen -W "--help" -- "$cur") ) ;;
            esac
            return 0
            ;;
        job)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "get list --help" -- "$cur") )
                return 0
            fi
            COMPREPLY=( $(compgen -W "--json --help" -- "$cur") )
            return 0
            ;;
        plugin)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "sync debug --help" -- "$cur") )
                return 0
            fi
            case "$cmd2" in
                debug) COMPREPLY=( $(compgen -W "--json --help" -- "$cur") ) ;;
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
                *)             COMPREPLY=( $(compgen -W "--help" -- "$cur") ) ;;
            esac
            return 0
            ;;
        user)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "get list --help" -- "$cur") )
                return 0
            fi
            COMPREPLY=( $(compgen -W "--json --help" -- "$cur") )
            return 0
            ;;
    esac
}

complete -F _priorisea priorisea
