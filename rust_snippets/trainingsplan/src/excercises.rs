use serde::{Deserialize, Serialize};
use serde_json::Result;
use std::collections::HashMap;
use std::collections::HashSet;
use std::fmt::Write;

#[derive(PartialEq, Eq, Hash, Clone, Serialize, Deserialize, Debug)]
pub enum ExcerciseValueType {
    Duration=0,     //in Seconds
    Weight=1,       //in Kg
    Repetition=2,
}

impl ExcerciseValueType {
    pub fn names() -> [&'static str; 3] {
        return ["Duration", "Weight", "Repetition"];
    }

    pub fn values() -> [ExcerciseValueType; 3] {
        return [ExcerciseValueType::Duration, ExcerciseValueType::Weight, ExcerciseValueType::Repetition];
    }

    pub fn from_string(excercise_type: &str) -> Option<ExcerciseValueType> {
        if( excercise_type == "Duration"  ) {
            return Some(ExcerciseValueType::Duration);
        }
        else if( excercise_type == "Weight" ) {
            return Some(ExcerciseValueType::Weight);
        }
        else if( excercise_type == "Repetition" ) {
            return Some(ExcerciseValueType::Repetition);
        }
        else {
            return None;
        }
    }
}

impl std::fmt::Display for ExcerciseValueType {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{:?}", self)
    }
}

#[derive(Serialize,Deserialize, Clone)]
pub struct ExcerciseTemplate {
    pub value_type: HashSet<ExcerciseValueType>,
    pub name: String,
}

impl ExcerciseTemplate {
    pub fn new(name: &String, value_type: &HashSet<ExcerciseValueType>) -> ExcerciseTemplate {
        ExcerciseTemplate{name: name.clone(), value_type: value_type.clone()}
    }
}

impl std::fmt::Display for ExcerciseTemplate {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        let mut tmp = String::new();

        write!(tmp, "name={} value_types=", self.name);

       
        for i in &self.value_type {
            write!(tmp, "{},", i);
        }

        write!(f, "{}", tmp)
    }
}


#[derive(Serialize,Deserialize)]
pub struct Excercise {
    pub excercise_template: ExcerciseTemplate,
    pub duration:   Option<u32>,
    pub weight:     Option<u32>,
    pub repetition: Option<u32>,
}

impl Excercise {
    pub fn get_value(&self, value_type: &ExcerciseValueType) -> Option<u32> {
        match value_type {
            ExcerciseValueType::Duration =>   { return self.duration },
            ExcerciseValueType::Weight =>     { return self.weight },
            ExcerciseValueType::Repetition => { return self.repetition },
        }
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

        $m.insert(($name).to_string(), ExcerciseTemplate::new(&String::from($name), &typeset));
    };
}

pub fn get_excercices() -> HashMap<String, ExcerciseTemplate> {
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