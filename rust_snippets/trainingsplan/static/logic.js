"use strict"

function create_excercise_entry(aTemplate)
{
    let li      = $('<li></li>')
    $(li).addClass("excercise_item");
    
    //excercise name
    let excercise_name = $('<b></b>').text(aTemplate.name);
    li.append(excercise_name);

    aTemplate.data.value_type.forEach( function(aValueType) {
        console.log(aValueType);
        if(aValueType == 'Duration') {
            let duration_name  = document.createTextNode('Duration: ');
            let duration_input = $('<input type="number" />') ;
            let duration_unit  = document.createTextNode('Seconds');

            li.append(duration_name);
            li.append(duration_input);
            li.append(duration_unit);

            $(duration_input).addClass("excercise_input_duration");
        } 
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
       
        let post_data = {
            'id': 1357
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
          });
    });
}