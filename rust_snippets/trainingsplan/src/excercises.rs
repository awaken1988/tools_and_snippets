use serde::{Deserialize, Serialize};
use serde_json::Result;
use std::collections::HashMap;
use std::collections::HashSet;

#[derive(PartialEq, Eq, Hash, Clone, Serialize)]
pub enum ExcerciseValueType {
    Duration=0, 
    Weigth=1,
    Repetition=2,
}

#[derive(Serialize)]
pub struct Excercise {
    pub value_type: HashSet<ExcerciseValueType>,
    pub name: &'static str,

}

impl Excercise {
    pub fn new(name: &'static str, value_type: &HashSet<ExcerciseValueType>) -> Excercise {
        Excercise{name: name, value_type: value_type.clone()}
    }
}

macro_rules! insert_excercise_type {
    ($typeset:expr, $type0:expr) => {
        $typeset.insert($type0)
    };
    ($typeset:expr, $type0:expr, $($typex:expr),+) => {
        $typeset.insert($type0);
        insert_excercise_type!($typeset, $($typex),+);
    };
}

macro_rules! insert_excercise {
    ($m:expr, $name:expr, $($types:expr),+) => {
        if $m.contains_key($name) {
            panic!("Duplicates")
        }

        let mut typeset = HashSet::new();

        insert_excercise_type!(typeset, $($types),+);

        $m.insert($name, Excercise::new($name, &typeset));
    };
}

pub fn get_excercices() -> HashMap<&'static str, Excercise> {
    let mut ret = HashMap::new();

    insert_excercise!(ret, "Einbeinstand",             ExcerciseValueType::Duration);          
    insert_excercise!(ret, "Balance Kniebeuge",        ExcerciseValueType::Duration);              
    insert_excercise!(ret, "Pilates Schulter",         ExcerciseValueType::Duration);              
    insert_excercise!(ret, "Superman",                 ExcerciseValueType::Duration);      
    insert_excercise!(ret, "Blank mit Bewegung",       ExcerciseValueType::Duration);              
    insert_excercise!(ret, "Ausfallschritt",           ExcerciseValueType::Duration);          
    insert_excercise!(ret, "Handlauf",                 ExcerciseValueType::Duration);      
    insert_excercise!(ret, "Dehnen Bein Vorn/Hinten",  ExcerciseValueType::Duration);                  
    insert_excercise!(ret, "Dehnen Oberkörper",        ExcerciseValueType::Duration);              
    insert_excercise!(ret, "Kobra",                    ExcerciseValueType::Duration);  
    insert_excercise!(ret, "Hängen",                   ExcerciseValueType::Duration);  
    insert_excercise!(ret, "Stocktraining",            ExcerciseValueType::Duration);          
    insert_excercise!(ret, "Freies Traing",            ExcerciseValueType::Duration);          
    insert_excercise!(ret, "Terraband Rücken",         ExcerciseValueType::Duration);              
    insert_excercise!(ret, "Terraband Brust",          ExcerciseValueType::Duration);          
    insert_excercise!(ret, "Terraband Schulter",       ExcerciseValueType::Duration);              
    insert_excercise!(ret, "Terraband Bizeps/Trizeps", ExcerciseValueType::Duration);                      

    ret
}