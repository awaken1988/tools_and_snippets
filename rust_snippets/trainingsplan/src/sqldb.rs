use crate::excercises;
use std::fmt::Write;
use std::collections::{HashSet,HashMap};
use serde::{Deserialize, Serialize};

use crate::excercises::{ExcerciseTemplate,ExcerciseValueType};

//TODO: make a proper typ in excercises.rs
#[derive(Serialize,Deserialize,Debug)]
pub struct ExcerciseSync {
    pub id:         u32,

    #[serde(default)]
    pub duration:   Option<u32>,
    
    #[serde(default)]
    pub weight:     Option<u32>,
    
    #[serde(default)]
    pub repetition: Option<u32>,
}



struct ColumnQuery {
    columns:       Vec<String>,
    columns_value: Vec<String>,
}

impl ColumnQuery {
    fn new() -> ColumnQuery {
        ColumnQuery {
            columns:       Vec::new(),
            columns_value: Vec::new(),
        }
    }

    fn add_name(mut self, name: &str) -> ColumnQuery {
        self.columns.push(name.to_string());
        self
    }

    fn add_name_value(mut self, name: &str, value: &str) -> ColumnQuery {
        self.columns.push(name.to_string());
        self.columns_value.push(value.to_string());
        self
    }

    fn add_list<'a, T>(mut self, names: T) -> ColumnQuery where
        T: IntoIterator<Item = &'a str>
    {
        for i in names {
            self.columns.push(i.to_string());
        }

        self
    }

    fn result_names(&self) -> String {
        let mut ret = String::new();
        for (idx, name) in self.columns.iter().enumerate() {
            if(idx > 0 ) {
                write!(ret, ",");
            }
            write!(ret, "{}", name);
        }
        return ret;
    }

    fn result_values(&self) -> String {
        let mut ret = String::new();
        for (idx, name) in self.columns_value.iter().enumerate() {
            if(idx > 0 ) {
                write!(ret, ",");
            }
            write!(ret, "{}", name);
        }
        return ret;
    }

    fn get(&self, idx: usize) -> String {
        self.columns.get(idx).unwrap().clone()
    }
}

pub fn create_db()
{
    let db = open();

    db.close().unwrap();

    get_excercise_template();
}

pub fn get_excercise_template() -> HashMap<String, ExcerciseTemplate>
{
    let db = open();

    let columns = ColumnQuery::new()
        .add_name("id")
        .add_name("name")
        .add_list(ExcerciseValueType::names());


    let query = format!("SELECT {} from excercise_template", columns.result_names());

    let mut stmt    = db.prepare(query.as_str()).unwrap();
    let result_iter = stmt.query_map([], |row| {
        let mut all_types = HashSet::<ExcerciseValueType>::new();

        let offset: usize = 2;

        //TODO: magic values
        for i in offset..(offset+ExcerciseValueType::names().len()) {
            if let Ok(value) = rusqlite::Row::get::<usize, u32>(row,i) {
                println!("{:?}", all_types);
                if(value == 1) {
                    all_types.insert( ExcerciseValueType::from_string(&columns.get(i)).unwrap() );
                }
            }
        }

        println!("{:?}", all_types);

        let name = rusqlite::Row::get::<usize, String>(row,1).unwrap();
        let id   = rusqlite::Row::get::<usize, u32>(row,0).unwrap();

        Ok(ExcerciseTemplate::new(id, &name, &all_types))
    }).unwrap();

    let mut ret = HashMap::new();

    for i_row in result_iter {
        let i_row_unwrapped = i_row.unwrap();

        println!("get_excercise_template {}", i_row_unwrapped.to_string());

        ret.insert(i_row_unwrapped.name.clone(), i_row_unwrapped.clone());
    }

    return ret;
}

pub fn write_excercise_log(log_entry: &ExcerciseSync )
{
    let db = open();

    let mut columns = ColumnQuery::new();

    //find excercise_template foreing key
    columns = columns.add_name_value("template_id", &format!("{}", log_entry.id)  );
   
    //TODO: use more generic way, we have the value names as array
    //  for now its hardcoded
    if let Some(x) = log_entry.duration {
        columns = columns.add_name_value("duration", &format!("{}", x));
    }
    if let Some(x) = log_entry.repetition {
        columns = columns.add_name_value("repetition", &format!("{}", x));
    }
    if let Some(x) = log_entry.weight {
        columns = columns.add_name_value("weight", &format!("{}", x));
    }
    
    let insert_sql = format!("INSERT INTO excercise_log({}) VALUES({})", columns.result_names(), columns.result_values());
    println!("write_excercise_log: {}", insert_sql);
    db.execute(&insert_sql, []).unwrap();
}

pub fn open() -> rusqlite::Connection {
    let basedir = std::fs::canonicalize("./runtime").unwrap();
    let dbfile  = basedir.as_path().join("main.sqlite3");

    std::fs::create_dir_all(basedir).unwrap();

    let is_created      = dbfile.is_file();

    let  dbcon = rusqlite::Connection::open_with_flags(
        dbfile, 
        rusqlite::OpenFlags::SQLITE_OPEN_CREATE | rusqlite::OpenFlags::SQLITE_OPEN_READ_WRITE)
        .unwrap();

    if !is_created {
        let mut fields = String::new();

        for (idx,name) in excercises::ExcerciseValueType::names().iter().enumerate() {
            if(idx != 0) {
                fields += ", ";
            }

            fields += &format!("{} INTEGER", &name);
        }


        //template of all excercises
        //Note: here we use fields as boolean
        {
            let excercice_template = format!("CREATE TABLE excercise_template(id INTEGER PRIMARY KEY, name TEXT NOT NULL, {})", &fields);
            println!("{}",&excercice_template);
            dbcon.execute(&excercice_template, []).unwrap();
        }
        
        //exercises log
        //Note: here we use fields as value e.g Kg
        {
            let excercice_log = format!("CREATE TABLE excercise_log(log_id INTEGER PRIMARY KEY, template_id INTEGER, {}, FOREIGN KEY(template_id) REFERENCES excercise_template(id) )", &fields);
            println!("{}",&excercice_log);
            dbcon.execute(&excercice_log, []).unwrap();
        }

        //fill db with value
        {
            for (key,val) in excercises::get_excercices() {
                let mut insert_name  = String::new();
                let mut insert_value = String::new();

                write!(insert_name, "name");
                write!(insert_value, "'{}'", key);

                for (idx,value_type) in excercises::ExcerciseValueType::values().iter().enumerate() {
                    write!(insert_name, ", ");
                    write!(insert_value, ",");
                    insert_name.push_str(&value_type.to_string());
                    insert_value.push_str(if val.value_type.contains(value_type) {"1"} else {"0"});
                }
                

                let sql_insert = format!("INSERT INTO excercise_template({}) VALUES ({})", insert_name, insert_value); 
                println!("{}", sql_insert);
                dbcon.execute(
                    sql_insert.as_str(), 
                    []).unwrap();
            }
        }
    }
    
    dbcon
}