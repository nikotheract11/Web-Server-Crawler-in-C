cd log
counter=0
for file in *
do
  t=$(grep 'search' $file | wc -l)
  let "counter=counter+$t"
  awk -F ':' '!/NE/' $file | cut -d ':' -f 5,6 >> temp1
done

m=$(cut -d ':' -f 1 temp1 | LANG=C sort | uniq -c | LANG=C sort | tail -1)
l=$(cut -d ':' -f 1 temp1 | LANG=C sort | uniq -c | LANG=C sort | head -1)
wordm=$(echo $m | cut -f 2 -d ' ')
wordl=$(echo $l | cut -f 2 -d ' ')
files=$(grep "$wordm" temp1 | LANG=C sort | uniq | wc -l)
filesl=$(grep "$wordl" temp1 | LANG=C sort | uniq | wc -l)

echo Total number of search commands : $counter
echo Keyword most frequently found: $wordm [totalNumFilesFound: $files]
echo Keyword least frequently found: $wordl [totalNumFilesFound: $filesl]
