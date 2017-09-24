grammar demo;

addition: addition PLUS ZAHL 
        | ZAHL;   

ZAHL: [0-9]+;
PLUS: '+';