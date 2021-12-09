
function remove_inputfile(id_, event) {
  event.preventDefault()

  confirm_modal("Remove Input File", "Removing an input file will remove it from the view, but not the file system. Do you still want to remove this file? ", "Yes","No", (e,m) =>{

     if (e) {
        $( "#inputfile-" + id_).remove();
        $.post( "/inputfiles/delete/" + id_, function( data ) {
            m.modal('hide')
        });
     }
  })

  }

function add_input_file(event) {
   $('#new_inputfile_modal').modal()
   event.stopPropagation()
   event.preventDefault();
}



function save_input_config(fileid, elm) {
    $.post("/inputfiles/configure/" + fileid, elm.serialize(), function(data) {
        $('#config_content_config').html(data)
        $.get("/inputfiles/update_main_header/" + fileid, function(data) {
            $("#main_header").html(data);
        })
    })
}


function close_inputfile_connection(fileid) {
    $.post("/inputfiles/disconnect/" + fileid, function(data) {
        $('#config_content_config').html(data)
    })
}


function show_load_input_file_modal(fileid) {
    $.get("/inputfiles/connected/" + fileid , function(data,s,xhr) {
        if (xhr.status == 200) {
            $('#config_input_load').modal('show')
        } else {
            alert("You must open a valid connection before loading an input file. ")
        }
    })
}

function load_input_file(fileid,felm) {
    $.post("/inputfiles/load_input_file/" + fileid, felm.serialize(), function(data) {
        $('#inputfile').val(data)
    })
}

function save_input_file(fileid,elm) {
    $.post("/inputfiles/save_input_file/" + fileid, elm.serialize(), function(data, s, xhr) {
         if (xhr.status == 200) {
            addToast("Save Successfull", "The input file was saved successfully", 5000)
         } else {
            alert("Something went wrong - Please try again.")
         }
    })

}

function show_load_spec_file_modal(fileid) {
    $.get("/inputfiles/connected/" + fileid , function(data,s,xhr) {
        if (xhr.status == 200) {
            $('#config_spec_load').modal('show')
        } else {
            alert("You must open a valid connection before loading a specification. ")
        }
    })
}


function load_spec_file(fileid,elm) {
    $.post("/inputfiles/load_spec/" + fileid, elm.serialize(), function(data) {
        $('#inputspec').val(data)
    })
}

function save_input_spec(fileid,elm) {
 $.post("/inputfiles/save_spec/" + fileid, elm.serialize(), function(data, s, xhr) {
         if (xhr.status == 200) {
            addToast("Save Successfull", "The specification file was saved successfully", 5000)
         } else {
            alert("Something went wrong - Please try again.")
         }
    })
}


function show_load_exec(fileid) {
    $.get("/inputfiles/connected/" + fileid , function(data,s,xhr) {
        if (xhr.status == 200) {
            $('#config_exec_load').modal('show')
        } else {
            alert("You must open a valid connection before loading a execution template.")
        }
    })
}

function load_exec_file(fileid,elm) {
   $.post("/inputfiles/load_exec/" + fileid, elm.serialize(), function(data) {
        $('#inputexec').val(data)
    })
}

function save_exec_file(fileid,elm) {
   $.post("/inputfiles/save_exec/" + fileid, elm.serialize(), function(data, s, xhr) {
         if (xhr.status == 200) {
            addToast("Save Successfull", "The execution file was saved successfully", 5000)
         } else {
            alert("Something went wrong - Please try again.")
         }
    })
}


function execute_file(fileid) {
   $('.input-save-button').click();
   alert("Execution Is Not Implemented Yet")
}
