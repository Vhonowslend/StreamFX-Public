#!/bin/bash

readarray -t hooks < <(find .hooks/ -maxdepth 1 -type d -not -wholename .hooks/ -print0)
for v in "${hooks[@]}"; do
	hookname=`basename $v`
	echo "#!/bin/bash" > .git/hooks/${hookname}
	echo "find .hooks/${hookname}/ -type f -name '*.sh' -exec '{}' \;" >> .git/hooks/${hookname}
done
