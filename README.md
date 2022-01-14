# IOS - first project

A project in the second semester at BUT FIT. 

______________________________________


The task was to create a script for the analysis of the stock exchange system record. The script will filter records and provide statistics specified by the user.

### Script specification
#### NAME
tradelog - analyzer of logs from stock exchange trading

#### USE

`tradelog [-h | --help] [FILTER] [COMMAND] [LOG [LOG2 [...]]`

#### OPTIONS

The COMMAND can be one of these:
- `list-tick` - a list of occurring stock exchange symbols, so-called "ticks"
- `profit` - statement of total profit from closed positions
- `pos` - list of values of currently held positions sorted in descending order by value
- `last-price` - a listing of the last known price for each ticker
- `hist-ord` - list of histogram of the number of transactions according to the ticker
- `graph-pos` - list of values of held positions according to the ticker

The FILTER can be a combination of these:
- `-a DATETIME` - after: only PO records on this date are considered (without this date). DATETIME is in the format YYYY-MM-DD HH: MM: SS
- `-b DATETIME` - before: only records BEFORE this date (without this date) are considered
- `-t TICKER` - only records corresponding to the given ticker are considered. With multiple occurrences of the switch, the set of all listed ticker is taken
- `-w WIDTH` - sets the width of the graph listing, ie the length of the longest line to WIDTH. Thus, WIDTH must be a positive integer. Multiple occurrences of the switch is a faulty start
- `-h and` --help print help with a brief description of each command and switch

______________________________________


Score: 15/15
