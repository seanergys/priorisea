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
    local cmd3=${words[3]:-}

    case "$prev" in
        --config*)
            COMPREPLY=( $(compgen -f -- "$cur") )
            return 0
            ;;
    esac

    if [ "$cmd1" = formula ] && [ "$cmd2" = set ]; then
        case "$prev" in
            -d|--default) return 0 ;;
        esac
    fi

    if [ "$cmd1" = timewin ] && { [ "$cmd2" = add ] || [ "$cmd2" = edit ]; }; then
        case "$prev" in
            -s|--score|-d|--duration|-c|--cron) return 0 ;;
        esac
    fi

    if [ "$cmd1" = score ] && [ "$cmd2" = job ]; then
        case "$prev" in
            -n|--limit) return 0 ;;
            --id-format)
                COMPREPLY=( $(compgen -W "dec hex kvs dothex words f58 f58plain" -- "$cur") )
                return 0
                ;;
        esac
    fi

    if [ "$cword" -eq 1 ]; then
        COMPREPLY=( $(compgen -W "init config formula decay score timewin plugin --config -h --help" -- "$cur") )
        return 0
    fi

    case "$cmd1" in
        init)
            COMPREPLY=( $(compgen -W "-h --help" -- "$cur") )
            return 0
            ;;
        config)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "get export -h --help" -- "$cur") )
                return 0
            fi
            case "$cmd2" in
                get)    COMPREPLY=( $(compgen -W "--json -h --help" -- "$cur") ) ;;
                export) COMPREPLY=( $(compgen -f -- "$cur") ) ;;
            esac
            return 0
            ;;
        formula)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "init list get set reset -h --help" -- "$cur") )
                return 0
            fi
            case "$cmd2" in
                init)  COMPREPLY=( $(compgen -W "-h --help" -- "$cur") ) ;;
                list)  COMPREPLY=( $(compgen -W "--json -h --help" -- "$cur") ) ;;
                get)   COMPREPLY=( $(compgen -W "--json -h --help" -- "$cur") ) ;;
                set)   COMPREPLY=( $(compgen -W "-d --default --json -h --help" -- "$cur") ) ;;
                reset) COMPREPLY=( $(compgen -W "-h --help" -- "$cur") ) ;;
            esac
            return 0
            ;;
        decay)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "init get set reset -h --help" -- "$cur") )
                return 0
            fi
            case "$cmd2" in
                init)  COMPREPLY=( $(compgen -W "-h --help" -- "$cur") ) ;;
                get)   COMPREPLY=( $(compgen -W "--json -h --help" -- "$cur") ) ;;
                set)   COMPREPLY=( $(compgen -W "--json -h --help" -- "$cur") ) ;;
                reset) COMPREPLY=( $(compgen -W "-h --help" -- "$cur") ) ;;
            esac
            return 0
            ;;
        score)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "user job update init -h --help" -- "$cur") )
                return 0
            fi
            case "$cmd2" in
                user)   COMPREPLY=( $(compgen -W "--json --all -h --help" -- "$cur") ) ;;
                job)    COMPREPLY=( $(compgen -W "--json --all -n --limit --id-format -h --help" -- "$cur") ) ;;
                update) COMPREPLY=( $(compgen -W "--new -h --help" -- "$cur") ) ;;
                init)   COMPREPLY=( $(compgen -W "-h --help" -- "$cur") ) ;;
            esac
            return 0
            ;;
        timewin)
            if [ "$cword" -eq 2 ]; then
                COMPREPLY=( $(compgen -W "list get add delete edit -h --help" -- "$cur") )
                return 0
            fi
            case "$cmd2" in
                list)   COMPREPLY=( $(compgen -W "--json -h --help" -- "$cur") ) ;;
                get)    COMPREPLY=( $(compgen -W "-j --json -h --help" -- "$cur") ) ;;
                add)    COMPREPLY=( $(compgen -W "-s --score -c --cron -d --duration --json -h --help" -- "$cur") ) ;;
                delete) COMPREPLY=( $(compgen -W "-h --help" -- "$cur") ) ;;
                edit)   COMPREPLY=( $(compgen -W "-s --score -c --cron -d --duration --json -h --help" -- "$cur") ) ;;
            esac
            return 0
            ;;
        plugin)
            if [ "$cword" -eq 2 ]; then
                    COMPREPLY=( $(compgen -W "seanerbus -h --help" -- "$cur") )
                return 0
            fi
            if [ "$cmd2" = seanerbus ]; then
                if [ "$cword" -eq 3 ]; then
                    COMPREPLY=( $(compgen -W "get set -h --help" -- "$cur") )
                    return 0
                fi
                case "$cmd3" in
                    get) COMPREPLY=( $(compgen -W "--json -h --help" -- "$cur") ) ;;
                    set) COMPREPLY=( $(compgen -W "-h --help" -- "$cur") ) ;;
                esac
                return 0
            fi
            return 0
            ;;
    esac
}

complete -F _priorisea priorisea
