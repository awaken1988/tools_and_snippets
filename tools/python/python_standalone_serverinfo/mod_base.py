import uuid

class ModBase:
    def __init__(self):
        self.uuid = str(uuid.uuid4()).replace("-", "_")

    def content(self):
        content =  r"<div class='"+self.uuid+"'>"
        content += r"<script>"
        content += r"    $().ready( $.get( '/?moduuid="+self.uuid+"', function( data ) {"
        content += r"        $( '."+self.uuid+"' ).html( data );"
        content += r"        }));"    
        content += r"</script>"
        content += r"</div>"

        return content
        