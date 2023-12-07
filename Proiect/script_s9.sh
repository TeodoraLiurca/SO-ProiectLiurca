# Verifică dacă numărul de argumente este 1
if [ "$#" -ne 1 ]; then
    echo "Utilizare: $0 <c>"
    exit 1
fi

character="$1"
counter=0
# Citirea continuă a liniilor până la end-of-file
while IFS= read -r line; do
    if [[ $line =~ ^[[:upper:]] ]]; then #conditie ca linie sa inceapa cu litera mare
        if [[ $line =~ ^[[:upper:][:lower:][:digit:][:space:],.!?]+[.!?]$ &&
		  $line != *,si* ]]; then
            ((counter++))
        fi
    fi
done
echo "$counter"
