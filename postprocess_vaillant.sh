sed -e 's/0\{100,\}/\n\n000000000000000000000000000000000000000000000000000000000000\n\n/g;s/1\(0\{5,\}\)1/1\n\1\n1/g' 
