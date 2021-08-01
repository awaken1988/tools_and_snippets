use actix_files as fs;
use actix_web::{App, HttpServer, Responder};

async fn index() -> impl Responder {
    "{\"squadName\": \"Super hero squad\"}"
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    HttpServer::new(|| {
        App::new()
            .service(fs::Files::new("/static", "./static/").show_files_listing())
            .route("/api/excercises", actix_web::web::get().to(index))
    })
    .bind("127.0.0.1:8080")?
    .run()
    .await
}