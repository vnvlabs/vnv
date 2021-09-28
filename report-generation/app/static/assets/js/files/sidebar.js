
function loading() {

}

function remove_file(id_, event, refresh) {
  event.preventDefault()
  act = refresh ? "Refresh" : "Remove"
  confirm_modal( act + " File", "Are you sure?" , "Yes","No", (e,m)=>{

     if (e) {

        url = "/files/delete/" + id_
        if (refresh) {
          url += "?refresh=true"
        } else {
            $( "#file-" + id_).remove();
        }

        $.post(url, function( data ) {
            if (refresh) {
                loading();
                window.location.href = data
            } else {
                m.modal('hide')
            }
        });
     }
  })

  }

function add_file(event) {
   $('#new_file_modal').modal()
   event.stopPropagation()
   event.preventDefault();
}

function start_processing_update(fileId, procTag) {
  $(window).on('load', function() {
        update_processing(fileId, procTag);
  });
}

function update_roles() {

   data = []
   elms = []
   $('[data-o="on"]').each(

     function(i){
        q = $(this).data('q')
        f = $(this).data('f')
        i = $(this).data('i')
        m = $(this).data('m')
        data.push([f,i,q,m])
        elms.push(this)
     }
   )
   if (data.length) {
     console.log(data)

     $.ajax({
        url: "/directives/roles" ,
        type: "POST",
        contentType: "application/json",
        data: JSON.stringify(data),
        success: function(response){
            for (i=0; i < response.length; i++) {
                if ( $(elms[i]).data('t') == "span" ) {
                    $(elms[i]).text(response[i][1])
                } else {
                    $(elms[i]).html(response[i][1])
                }
                if (!response[i][0]) {
                    $(elms[i]).attr('data-o','off');
                }
            }
            setTimeout(update_roles, 3000)
        }
     });


   } else {
      setTimeout(update_roles, 3000)
   }


}

$(window).on('load', function() {
    update_roles()
    setInterval(function() {
       MathJax.typeset();
    }, 3000)
})

function update_processing(fileId, procTag) {
   $.get('/files/processing/' + fileId, function(data, textStatus, xhr) {
        if (xhr.status == 201) {
            $(procTag).hide();
        } else {
            setTimeout(function(){ update_processing(fileId, procTag) }, 3000);
        }
   });
}

function switch_package(fileId, name, element) {
    $(".prov-selected").removeClass("prov-selected");
    $('#'+name).addClass("prov-selected")

    $.get("/files/viewers/package/"+fileId + "?p=" + name, function(data) {
      $(element).html(data)
    })

}

function switch_action(fileId, name, element) {
  $(".action-selected").removeClass("action-selected");
  $('#'+name).addClass("action-selected")

  $.get("/files/viewers/action/"+fileId + "?p=" + name, function(data) {
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

function update_soon(url,  containerId, timeout, chartupdate) {
    if ($('#' + containerId).length) {
        setTimeout(function() {
            $.get(url, function(response) {
               var x = JSON.parse(response)
               chartupdate(x.config)
               if (x.more) {
                 update_soon(url,  containerId, timeout,chartupdate)
               }
            })
        }, timeout)
    }
}
function update_request(fileId, ipid, elmid) {

   if ( $('#' + elmid ).length ) {
      url ="/files/viewers/iprequest/" + fileId + "/" + ipid
      $.get(url, function(data,status, xhr) {
        if (xhr.status == 200) {
           $('#' + elmid ).html(data)
        } else if (xhr.status == 201 ) {
           $('#' + elmid ).html("")
           setTimeout(function() {
              update_request(fileId, ipid, elmid)
           }, 3000)
        }
      })
    }

}

function  update_ip_processing(elm, file, ipid ) {
    if ( $('#' + elm ).length ) {
        url = "/files/viewers/processing/" + file + "/" + ipid
        $.get(url, function(response, status,  xhr) {
            $('#' + elm ).text(response)
            if (xhr.status == 200) {
                setTimeout(function() {
                   update_ip_processing(elm,file,ipid)
                },3000)
            }
        });
    }
}


function submit_form(elm, url, response) {
    $.ajax({
            url: url,
            type: 'post',
            data:$('#' + elm).serialize(),
            success: response
    });
}

function submit_response_form(containerId, ipFile, ipId ) {
    submit_form('steering_response_form', '/files/viewers/respond', function(data,stat,xhr) {
        $('#' + containerId).html("<div class='card' style='position:relative'><div class='card-body'><h3>Response Sent</h3></div></div>    ")
        setTimeout(function() {
          update_request(ipFile,ipId,containerId)
        }, 3000);
    });
}

function configure_response_validation(inputElmId, fid, ipid, action) {
  $('#' + inputElmId).bind('input propertychange', function() {
    $.ajax({
        url: '/files/viewers/validateResponse/' + fid + '/' + ipid ,
        type: "POST",
        contentType: "application/json",
        data: JSON.stringify({"data" : this.value }),
        success: function(response, stat, xhr){
           action(xhr.status == 200, response)
        }
     });
  })
}


$(window).on('load', function() {
    setInterval(function() {
       $('[data-countdown="on"').each(function () {
          $(this).text(parseInt($(this).text())-1)
       })
    }, 1000)
});

