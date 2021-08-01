use actix_files as fs;
use actix_web::{App, HttpServer, Responder};
use serde::{Deserialize, Serialize};
use serde_json::Result;

mod excercises;
use excercises::{*};

async fn excercise_list() -> impl Responder {
   //let des = [
   //    Excercise{name: "Einbeinstand",      value_type: ExcerciseValueType::Time},
   //    Excercise{name: "Balance Kniebeuge", value_type: ExcerciseValueType::Time},
   //];

   let des = get_exercices();

   serde_json::to_string(&des)
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    HttpServer::new(|| {
        App::new()
            .service(fs::Files::new("/static", "./static/").show_files_listing())
            .route("/api/excercise_list", actix_web::web::get().to(excercise_list))
    })
    .bind("127.0.0.1:8080")?
    .run()
    .await
}