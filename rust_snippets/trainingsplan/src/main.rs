use actix_files as fs;
use actix_web::{App, HttpServer, Responder, web};
use serde::{Deserialize, Serialize};
use serde_json::Result;
use actix_web::middleware::Logger;
use env_logger::Env;

use std::fs::File;
use std::fs::OpenOptions;
use std::io::prelude::*;

mod excercises;
use excercises::{*};

mod sqldb;

#[derive(Deserialize)]
struct ExcerciseAdd {
    name: String,

}

async fn excercise_list() -> impl Responder {
   let des = sqldb::get_excercise_template();

   serde_json::to_string(&des)
}

//TODO: make a proper typ in excercises.rs
#[derive(Serialize,Deserialize,Debug)]
struct ExcerciseSync {
    pub id:         u32,

    #[serde(default)]
    pub duration:   Option<u32>,
    
    #[serde(default)]
    pub weight:     Option<u32>,
    
    #[serde(default)]
    pub repetition: Option<u32>,
}

async fn excercise_add(info: web::Json<ExcerciseSync>) -> Result<String> {

//    let mut file = OpenOptions::new()
//        .create(true)
//        .write(true)
//        .append(true)
//        .open("out.txt")
//        .unwrap();
//    file.write_all(info.excercise_template.name.as_bytes()).unwrap();
//    file.write_all("\n".as_bytes()).unwrap();

//    sqldb::write_excercise_log(&info);

    println!("{:?}", info);

    Ok(format!("Welcome {}!",info.id))
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {

    //prepare db
    sqldb::create_db();


    //std::env::set_var("RUST_LOG", "info,actix_web=error");
    env_logger::from_env(Env::default().default_filter_or("info")).init();


    HttpServer::new(|| {
        App::new()
            .wrap(Logger::default())
            .wrap(Logger::new("%a %{User-Agent}i"))
            .service(fs::Files::new("/static", "./static/").show_files_listing())
            .route("/api/excercise_template_list", actix_web::web::get().to(excercise_list)) //TODO: rename to template_list
            .route("/api/excersice_add", actix_web::web::post().to(excercise_add))
    })
    .bind("127.0.0.1:8080")?
    .run()
    .await
}