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

#[derive(Deserialize)]
struct ExcerciseAdd {
    name: String,

}

async fn excercise_list() -> impl Responder {
   let des = get_excercices();

   serde_json::to_string(&des)
}

async fn excercise_add(info: web::Json<Excercise>) -> Result<String> {

    let mut file = OpenOptions::new()
        .create(true)
        .write(true)
        .append(true)
        .open("out.txt")
        .unwrap();
    file.write_all(info.excercise_template.name.as_bytes()).unwrap();
    file.write_all("\n".as_bytes()).unwrap();

    Ok(format!("Welcome {}!", info.excercise_template.name))
}

//#[actix_web::main]
//async fn main() -> std::io::Result<()> {
//    //std::env::set_var("RUST_LOG", "info,actix_web=error");
//    env_logger::from_env(Env::default().default_filter_or("info")).init();
//
//
//    HttpServer::new(|| {
//        App::new()
//            .wrap(Logger::default())
//            .wrap(Logger::new("%a %{User-Agent}i"))
//            .service(fs::Files::new("/static", "./static/").show_files_listing())
//            .route("/api/excercise_template_list", actix_web::web::get().to(excercise_list)) //TODO: rename to template_list
//            .route("/api/excersice_add", actix_web::web::post().to(excercise_add))
//    })
//    .bind("127.0.0.1:8080")?
//    .run()
//    .await
//}

//sqlite test
fn main() -> Result<()> {

    let basedir = std::fs::canonicalize("./runtime").unwrap();
    let dbfile  = basedir.as_path().join("main.sqlite3");

    std::fs::create_dir_all(basedir).unwrap();

    let is_created      = dbfile.is_file();

    let  dbcon = rusqlite::Connection::open_with_flags(
        dbfile, 
        rusqlite::OpenFlags::SQLITE_OPEN_CREATE | rusqlite::OpenFlags::SQLITE_OPEN_READ_WRITE)
        .unwrap();

    if !is_created {
        dbcon.execute("CREATE TABLE excercise_template(id INTEGER, name TEXT NOT NULL)", []).unwrap();
    }
    
    Ok(())
}