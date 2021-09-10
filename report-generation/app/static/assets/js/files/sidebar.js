
function remove_file(id_, event) {
  event.preventDefault()

  confirm_modal("Remove File", "Removing a file will remove it from the view, but not the file system. Do you still want to remove this file? ", "Yes","No", e=>{

     if (e) {
        $( "#file-" + id_).remove();
        $.post( "/files/delete/" + id_, function( data ) {});
     }
  })

  }

function add_file(event) {
   $('#new_file_modal').modal()
   event.stopPropagation()
   event.preventDefault();
}

function switch_package(fileId, name, element) {
    $(".prov-selected").removeClass("prov-selected");
    $('#'+name).addClass("prov-selected")

    $.get("/files/viewers/package/"+fileId + "?p=" + name, function(data) {
      $(element).html(data)
    })

}

function switch_pending(fileId, name, element) {
    $(".steering-selected").removeClass("steering-selected");
    $('#'+name).addClass("steering-selected")
    $.get("/steering/pending/"+fileId + "?p=" + name, function(data) {
      $(element).html(data)
    })
}

function switch_unit(fileId, name, id, element) {
    $(".unit-selected").removeClass("unit-selected");
    $('#'+name).addClass("unit-selected")

    $.get("/files/viewers/unit/"+fileId + "?uid=" + id, function(data) {
      $(element).html(data)
    })

}

function show_file_reader(filename, reader ) {
       $('#file_viewer_modal').modal('show')
       $('#file_viewer_modal_body').html("<div>Loading...</div>")
       $('#file_view_modal_dialog').css("max-width","90%")

       url = "/files/reader"
       url += "?reader=" + reader
       url += "&filename=" + filename

       $.get(url, function(data) {
            $('#file_viewer_modal').modal('hide')

            $('#file_viewer_modal').modal('hide')
            $('#file_viewer_modal_body').html(data)
            $('#file_viewer_modal').modal('show')


       });

}

function scroll_line_into_view(line) {
    $(".linenodiv:visible")[0].children[0].children[line].scrollIntoView({behavior: "smooth", block: "center", inline: "nearest"})
}
function scroll_to_bottom() {
    a = $(".linenodiv:visible")[0].children[0]
    a.children[ a.children.length -1 ].scrollIntoView({behavior: "smooth", block: "center", inline: "nearest"})
}

