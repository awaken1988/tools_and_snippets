use serde::{Deserialize, Serialize};
use serde_json::Result;
use std::collections::HashMap;

#[derive(Serialize)]
pub enum ExcerciseValueType {
    Time=0, 
    RepWeight=1,
}

#[derive(Serialize)]
pub struct Excercise {
    pub value_type: ExcerciseValueType,
    pub name: &'static str,

}

impl Excercise {
    pub fn new(name: &'static str, value_type: ExcerciseValueType) -> Excercise {
        Excercise{name: name, value_type: value_type}
    }
}

macro_rules! insert_excercise {
    ($m:expr, $l:expr) => {
        if $m.contains_key($l.name) {
            panic!("Duplicates")
        }

        $m.insert($l.name, $l)
    };
}

pub fn get_excercices() -> HashMap<&'static str, Excercise> {
    let mut ret = HashMap::new();

    insert_excercise!(ret, Excercise::new("Einbeinstand",             ExcerciseValueType::Time));          
    insert_excercise!(ret, Excercise::new("Balance Kniebeuge",        ExcerciseValueType::Time));              
    insert_excercise!(ret, Excercise::new("Pilates Schulter",         ExcerciseValueType::Time));              
    insert_excercise!(ret, Excercise::new("Superman",                 ExcerciseValueType::Time));      
    insert_excercise!(ret, Excercise::new("Blank mit Bewegung",       ExcerciseValueType::Time));              
    insert_excercise!(ret, Excercise::new("Ausfallschritt",           ExcerciseValueType::Time));          
    insert_excercise!(ret, Excercise::new("Handlauf",                 ExcerciseValueType::Time));      
    insert_excercise!(ret, Excercise::new("Dehnen Bein Vorn/Hinten",  ExcerciseValueType::Time));                  
    insert_excercise!(ret, Excercise::new("Dehnen Oberkörper",        ExcerciseValueType::Time));              
    insert_excercise!(ret, Excercise::new("Kobra",                    ExcerciseValueType::Time));  
    insert_excercise!(ret, Excercise::new("Hängen",                   ExcerciseValueType::Time));  
    insert_excercise!(ret, Excercise::new("Stocktraining",            ExcerciseValueType::Time));          
    insert_excercise!(ret, Excercise::new("Freies Traing",            ExcerciseValueType::Time));          
    insert_excercise!(ret, Excercise::new("Terraband Rücken",         ExcerciseValueType::Time));              
    insert_excercise!(ret, Excercise::new("Terraband Brust",          ExcerciseValueType::Time));          
    insert_excercise!(ret, Excercise::new("Terraband Schulter",       ExcerciseValueType::Time));              
    insert_excercise!(ret, Excercise::new("Terraband Bizeps/Trizeps", ExcerciseValueType::Time));                      

    ret
}