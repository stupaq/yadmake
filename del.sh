for q in $( ipcs -q | cut -d " " -f 2)
do
	ipcrm -q "$q"
done
