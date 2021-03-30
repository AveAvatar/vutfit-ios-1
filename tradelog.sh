#! /bin/sh
# IOS - project 1
# author : Tadeáš Kachyňa
# login : xkachy00
# date : 2021-04-04
# usage : type --help or -h

export POSIXLY_CORRECT=yes
export LC_NUMERIC=en_US.UTF-8

# This function might help you, if you get lost
help()
{
    echo "Usage: tradelog [-h|--help] [FILTR] [PŘÍKAZ] [LOG [LOG2 [...]]"

    echo "One of the commands can be:"
    echo "  list-tick – listing of occurring stock exchange symbols, so-called tickers"
    echo "  profit – statement of total profit from closed positions"
    echo "  pos – list of values of currently held positions sorted in descending order by value"
    echo "  last-price – listing of the last known price for each ticker"
    echo "  hist-ord – histogram report of the number of transactions according to the ticker"
    echo "  graph-pos – statement of the graph of values of held positions according to the ticker"

    echo "The FILTER can be a combination of the following:"
    echo "  -a DATETIME – after: only records after this date are considered (without this date). DATETIME is in the format YYYY-MM-DD HH: MM: SS"
    echo "  -b DATETIME – before: only records BEFORE this date (without this date) are considered"
    echo "  -t TICKER – only entries corresponding to a given ticker are considered. With multiple occurrences of the switch, the set of all listed ticker is taken"
    echo "  -w WIDTH – in the list of graphs, sets their width, ie the length of the longest line to WIDTH. Thus, WIDTH must be a positive integer. Multiple occurrences of the switch is a faulty start"
}

# loading file(s)
for var in "$@"
do  
    
    if [ -f "$var" ]; then
    i=$((i+1))
    AB=$(cat "$var")
    LOG="${LOG} $AB"
    fi
done




# filtered tickers
TICKERS="$(echo "$LOG" | awk -F ';' '{print $2}' | sort -u)"

# flags
while getopts ":t:a:b:w:" opt; do
    case ${opt} in

    # -t TICKER – only entries corresponding to a given ticker are considered. With multiple occurrences of the switch, the set of all listed ticker is taken
    t)  
        IFS=" "
        TICKERSS="$TICKERSS $OPTARG"
      
        ;;

    # -a DATETIME – after: only records after this date are considered (without this date). DATETIME is in the format YYYY-MM-DD HH: MM: SS
    a)
        VARB=$(awk -F ';' '{print $1}' stock.log)
        IFS="
"
        for d2 in $VARB
        do
            if [ "$OPTARG" = "$d2" ]; then
                echo "same day"
            elif expr "$OPTARG" "<" "$d2" >/dev/null; then
                LOGG=$LOGG"\n"$(grep "$d2" stock.log | awk '{print $0}')
         
            fi
        done

        ;;

    # -b DATETIME – before: only records BEFORE this date (without this date) are considered
    b) 
        VARB=$(awk -F ';' '{print $1}' stock.log)
        IFS="
"
        for d2 in $VARB
        do
            if [ "$OPTARG" = "$d2" ]; then
                echo "same day"
            elif expr "$OPTARG" ">" "$d2" >/dev/null; then
                LOGGO=$LOGGO"\n"$(grep "$d2" stock.log | awk '{print $0}')
         
            fi
        done

        ;;

    # -w WIDTH – in the list of graphs, sets their width, ie the length of the longest line to WIDTH. Thus, WIDTH must be a positive integer. Multiple occurrences of the switch is a faulty start
    w)
        WItdH="$OPTARG"
       # i = int(width / maxCount * count)
      
        ;;
    :)
        # if -k flag, no value, OPTARG contains "k"; handle here
        echo "$OPTARG"
   
        ;;
    *)  
        echo "Hello WOrld"
        # sorted tickers 

        ;;
    esac
    LOGGA="$LOGG"
 
done
shift $((OPTIND -1))


if [ -z "$TICKERSS" ]
then
    TICKERS=$TICKERS 
else
    TICKERS=$(echo "$TICKERSS"  | xargs -n1 | sort | xargs)
fi


if [ "$LOGG" ] && [ "$LOGGA" ]
then
    UNIQUE=$(echo "$LOGG $LOGGO" | tr ' ' '\n' | sort | uniq -d)
    LOG="$UNIQUE"
fi



# the main loop - goes until the number of arguments is greater than zero
while [ "$#" -gt 0 ] ; do
    case "$1" in

    # LIST-TICK – listing of occurring stock exchange symbols, so-called tickers
    list-tick)

        for a in $TICKERS ; do
            echo "$a" 
        done
        shift
        ;;

    # PROFIT – statement of total profit from closed positions
    profit)
        
        # getting all sell transactions
        SELL=$(echo "$LOG" | awk -F ';' '$3 ~ /sell/ {sum += $4*$6} END { printf ("%0.2f\n", sum)}')

        # getting all buy transactions
        BUY=$(echo "$LOG" | awk -F ';' '$3 ~ /buy/ {sum += $4*$6} END {printf ("%0.2f\n", sum)}')

        # printing the result
        echo "$SELL - $BUY" | bc

        shift 
        ;;
    
    # LAST-PRICE – listing of the last known price for each ticke
    last-price)
    

        # search for the largest number in column no. 4
        for a in $TICKERS ; do LOGGA=${LOGGA}"\n"$(echo "$LOG" | awk -F ';' -v var="$a"  '$2 == var {print $0}') ; done
    
        LENGTH=$(echo "$LOGGA" | awk -F ';' '{for (i=1;i<=NF;i++) if (length($4)>max) max=length($4)} END{print max}')+1
    
        for a in $TICKERS ; do  
            # get all "$a" transactions => print the last one => align to the right by the longest number
            echo "$LOG" | awk -F ';' -v var="$a"  '$2 == var {print $0}' |  awk -F ';' -v var="$a" -v len="$LENGTH" '$2 ~ var{a=$0} END{ printf "%-10s : %*s\n", var, len, $4}'
     
        done
        shift
        ;;

    #POS – výpis hodnot aktuálně držených pozic seřazených sestupně dle hodnoty
    pos)
        TICKERSS="TSM PYPL AAPL"
       
        for a in $TICKERSS
        do  
            # getting bought stocks for each ticket and add it to a var "sum"
            POS_BUY=$(echo "$LOG" | awk -F ';' -v lvar="$a" '$2 ~ lvar && $3 ~ /buy/ {sum += $6} END {print sum}')

            # getting sold stocks for each ticket and add it to a var "sum"
            POS_SELL=$(echo "$LOG" | awk -F ';' -v lvar="$a" '$2 ~ lvar && $3 ~ /sell/ {sum += $6} END {print sum}')

            # "sold" - "bought"
            POS_RESULT="$( echo "$POS_BUY - $POS_SELL" | bc)"

            
        
            # getting the last transaction
            POS_LAST_TRANS="$(echo "$LOG" | grep "$a" | tail -n1 | awk -F ';' '{print $4}')"

            #echo "$VAR2"
            POS_RESULT2="$POS_RESULT2$(echo "$POS_RESULT*$POS_LAST_TRANS"| bc),"

        
        done
        
        
        

        POS_RESULT_3=$(echo "$TICKERSS" "$POS_RESULT2"| awk '{split($1,a,","); split($2,b,",");
        for (counter = 1; counter <= 3 ; counter++)

           print "",a[counter],":",b[counter],"";
        }')
        POS_RESULT_4=$(echo "$POS_RESULT_3" | sort -t: -nk2 -r) 
        echo "$POS_RESULT_4" | column -t 
        shift
        ;;  

    # hist-ord – výpis histogramu počtu transakcí dle tickeru.

    hist-ord)
        for a in $TICKERS
        do
            VARa=$(echo "$LOG"| grep "$a")

            VARb=$(echo "$VARa" | awk 'END{print NR}')
        
            VARc=$(echo "$VARb"| awk '{
            s = sprintf("%*s", $1, " ")
            gsub(/ /, "#", s)

            print s p
            }')

            printf "%-11s : %s\n" "$a" "$VARc"

        done
        shift
        ;;
    # graph-pos – výpis grafu hodnot držených pozic dle tickeru
    graph-pos)
        for a in $TICKERS
        do  
            # getting bought stocks for each ticket and add it to a var "sum"
            POS_BUY=$(echo "$LOG" | awk -F ';' -v lvar="$a" '$2 ~ lvar && $3 ~ /buy/ {sum += $6} END {print sum}')

            # getting sold stocks for each ticket and add it to a var "sum"
            POS_SELL=$(echo "$LOG" | awk -F ';' -v lvar="$a" '$2 ~ lvar && $3 ~ /sell/ {sum += $6} END {print sum}')

            # "sold" - "bought"
            POS_RESULT="$( echo "$POS_BUY - $POS_SELL" | bc)"

            
        
            # getting the last transaction
            POS_LAST_TRANS="$(echo "$LOG" | grep "$a" | tail -n1 | awk -F ';' '{print $4}')"

            #echo "$VAR2"
            POS_RESULT2=$(echo "$POS_RESULT*$POS_LAST_TRANS"| bc)
            POS_RESULT3=$(echo "$POS_RESULT2/100000"| bc),
            

            TEST=$(echo "$POS_RESULT3" | awk -v var="$POS_RESULT3" -v w="$WITDH" '{
            if (var >= 0)
            {
               s = sprintf("%*s", $1, " ")
               gsub(/ /, "#", s)

               print s p
            } else {
               s = sprintf("%*s", $1, " ")
               gsub(/ /, "!", s)

               print s p

            }   
            
            }')

            printf  "%-10s: %s\n" "$a" "$TEST"   

        
        done  
        shift
        ;;
    
    # This might help you, if you get lost
  
    -h)
        help
        exit 0
        ;;

     # This also might help you, if you get lost
    -help)
        help
        exit 0
        ;;
               
    *)  

        
        for a in $TICKERS
        do
        TESTB=$(echo "$LOG" | grep -w "$a")
        TESTA="$TESTA
$TESTB"
        done
        
        echo "$TESTA" | awk 'NF' | sort -k1
     
        ;;
        
    esac
shift "$i"
done