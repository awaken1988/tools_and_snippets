use serde::{Deserialize, Serialize};
use serde_json::Result;

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

pub fn get_exercices() -> Vec<Excercise> {
    vec! [
        Excercise{name: "Einbeinstand",      value_type: ExcerciseValueType::Time},
        Excercise{name: "Balance Kniebeuge", value_type: ExcerciseValueType::Time},
    ]
}