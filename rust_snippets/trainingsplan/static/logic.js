"use strict"

function create_excercise_entry(aTemplate)
{
    let li      = $('<li class="excersice-entry-li"></li>')
    $(li).addClass("excercise_item");
    $(li).data('excercise-template', aTemplate);

    //excercise name
    let excercise_name = $('<b></b>').text(aTemplate.name);
    li.append($('<div class="excersice_input_name"/>')
        .append(excercise_name));

    aTemplate.data.value_type.forEach( function(aValueType) {
        console.log(aValueType);

        let entry_value_name  = ""
        let entry_unit        = ""
        let entry_class       = ""

        if(aValueType == 'Duration') {
            entry_value_name  = "Duration"
            entry_unit        = "Seconds"
            entry_class       = "excercise_input_duration"
        }
        if(aValueType == 'Repetition') {
            entry_value_name  = "Repetition"
            entry_class       = "excercise_input_repetition"
        }
        if(aValueType == 'Weight') {
            entry_value_name = "Weight"
            entry_unit       = "Kg"
            entry_class      = "excercise_input_weight"
        }

        let curr_input = $('<input type="number" />') ;

      
        li.append($('<div class="excersice_input_value_name"/>')
            .append(document.createTextNode(entry_value_name)));
        li.append(curr_input);
        li.append($('<div class="excersice_input_unit"/>')
            .append(document.createTextNode('['+entry_unit+']')));
        

        $(curr_input).addClass(entry_class);
    });

    //remove
    let btn_remove = $('<input/>' , {
        type: 'button',
        value: 'Delete',
        on: {
            click: function(){
                $(this).parent().remove();
            }
        }
    });

    li.append(btn_remove);

    return li;
}

function sync_excercise()
{
    $('.excercise_item').each(function(){
        let li = this;
        let excercise_template = $(this).data('excercise-template');
        console.log( excercise_template );

        let post_data = {
            'id': excercise_template.data.id
        };

        $(this).children('[class^=excercise_input_]').each(function() {
            let excercise_name = $(this).attr('class').replace('excercise_input_', '')

            post_data[excercise_name] = parseInt($(this).val());
        });

        console.log( post_data );
        

        $.ajax({
            type: "POST",
            url: "/api/excersice_add",
            data: JSON.stringify(post_data),
            contentType : 'application/json',
            complete: function(aXhr, aStatus) {
                if( aStatus==='success' ) {
                    $(li).remove();
                }
            }
          });
    });
}