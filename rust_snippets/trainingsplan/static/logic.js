"use strict"

function create_excercise_entry(aTemplate)
{
    let li      = $('<li></li>')
    $(li).addClass("excercise_item");
    $(li).data('excercise-template', aTemplate);

    //excercise name
    let excercise_name = $('<b></b>').text(aTemplate.name);
    li.append(excercise_name);

    aTemplate.data.value_type.forEach( function(aValueType) {
        console.log(aValueType);
        if(aValueType == 'Duration') {
            let duration_input = $('<input type="number" />') ;

            li.append( document.createTextNode('Duration: '));
            li.append(duration_input);
            li.append(document.createTextNode('[Seconds]'));
            li.append(document.createTextNode(';    '));

            $(duration_input).addClass("excercise_input_duration");
        }
        if(aValueType == 'Repetition') {
            let repetition_input = $('<input type="number" />') ;

            li.append(document.createTextNode('Repetition: '));
            li.append(repetition_input);
            li.append(document.createTextNode(';    '));

            $(repetition_input).addClass("excercise_input_repetition");
        }
        if(aValueType == 'Weight') {
            let weight_input = $('<input type="number" />') ;

            li.append(document.createTextNode('Weight: '));
            li.append(weight_input);
            li.append(document.createTextNode('[Kg]'));
            li.append(document.createTextNode(';    '));

            $(weight_input).addClass("excercise_input_weight");
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