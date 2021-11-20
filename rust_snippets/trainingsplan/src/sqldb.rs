use crate::excercises;
use std::fmt::Write;
use std::collections::HashSet;

use crate::excercises::{ExcerciseTemplate,ExcerciseValueType};

struct ColumnQuery {
    columns: Vec<String>,
}

impl ColumnQuery {
    fn new() -> ColumnQuery {
        ColumnQuery {
            columns: Vec::new(),
        }
    }

    fn add(mut self, name: &str) -> ColumnQuery {
        self.columns.push(name.to_string());
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

    fn result(&self) -> String {
        let mut ret = String::new();
        for (idx, name) in self.columns.iter().enumerate() {
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

pub fn get_excercise_template() 
{
    let db = open();

    let columns = ColumnQuery::new()
        .add("name")
        .add_list(ExcerciseValueType::names());


    let query = format!("SELECT {} from excercise_template", columns.result());

    let mut stmt    = db.prepare(query.as_str()).unwrap();
    let result_iter = stmt.query_map([], |row| {
        let mut all_types = HashSet::<ExcerciseValueType>::new();

        for i in 1..3 {
            if let Ok(value) = rusqlite::Row::get::<usize, u32>(row,i) {
                if(value == 1) {
                    all_types.insert( ExcerciseValueType::from_string(&columns.get(i)).unwrap() );
                }
            }
        }

        let name = rusqlite::Row::get::<usize, String>(row,0).unwrap();

        Ok(ExcerciseTemplate::new(&name, &all_types))
    }).unwrap();

    for i in result_iter {
        println!("mydb content {}", i.unwrap().name)
    }

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