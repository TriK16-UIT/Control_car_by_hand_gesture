function isCameraOn()
{
    if(document.getElementById("Cameracheckbox").checked === false){
        disableCamera();
      } else { 
        enableCamera();
      }
}
var tracks;
function enableCamera()
{
    let video = document.querySelector("#videoElement");
    if (navigator.mediaDevices.getUserMedia)
    {
        navigator.mediaDevices.getUserMedia({video:true})
            .then(function (stream)
            {
                video.srcObject = stream;
                video.play();
            })
            .catch(function (stream)
            {
                console.log("Something went wrong!");
            })
    }
    else
    {
        console.log("getUserMedia not supported");
    }
}   
function disableCamera()
{
    let video = document.querySelector("#videoElement");
    if (navigator.mediaDevices.getUserMedia)
    {
        navigator.mediaDevices.getUserMedia({video:true})
            .then(function (stream)
            {
                stream.getTracks().forEach(function(track) {
                    track.stop();
                  });
                video.srcObject = null;              
            })
            .catch(function (stream)
            {
                console.log("Something went wrong!");
            })
    }
}
function capture()
{
    if(document.getElementById("Cameracheckbox").checked === true){
        var canvas = document.getElementById('canvas');
        var video = document.querySelector("#videoElement");
        canvas.getContext('2d').drawImage(video, 0, 0, 500, 375);
        canvas.style.display="none"
        s = JSON.stringify(canvas.toDataURL().split(';base64,')[1]);
        $.ajax({
            url:"/test",
            type:"POST",
            contentType: "application/json",
            data: s,
        }).done(function (s){
            document.getElementById("result").innerHTML = s;
        }).fail(function (error){
        });
    } else{
        document.getElementById("result").innerHTML = "Waiting for camera is on...";
    }
}
function ScanforBluetoothDevices()
{
    $.ajax({
        url:"/Scan",
        type:"POST",
        contentType: "application/json",
    }).done(function (result){
        var list = document.getElementById('List');
        list.innerHTML="";
        if (result.length !== 0){
            for (var i=0; i < result.length; ++i){
                var li = document.createElement('li');
                if (Object.values(result[i])[1] == ""){
                    li.innerHTML = `${Object.values(result[i])[0]} - Unknown Device`;
                    $(li).data('MAC', `${Object.values(result[i])[0]}`);
                    li.onclick = selectBluetoothDevice;
                    list.appendChild(li);
                } else{
                    li.innerHTML = `${Object.values(result[i])[0]} - ${Object.values(result[i])[1]}`;
                    $(li).data('MAC', `${Object.values(result[i])[0]}`);
                    li.onclick = selectBluetoothDevice;
                    list.appendChild(li);    
                }
            }
        } else{
            list.innerHTML="No device is found";
        }
    }).fail(function (error){
    });
}
function selectBluetoothDevice()
{
    var connect = document.getElementById('Connect');
    $(connect).data('MAC', `${$(this).data('MAC')}`);
    Array.prototype.slice.call(document.querySelectorAll('#List li')).forEach(function(element){
        element.classList.remove('selected');
      });
    this.classList.add('selected');
}
function ConnectBluetoothDevice()
{
    s = JSON.stringify($("button#Connect").data('MAC'));
    $.ajax({
        url:"/ConnectBluetooth",
        type:"POST",
        contentType: "application/json",
        data: s,
    }).done(function (result){
        alert(result);
    }).fail(function (error){
    });
}
setInterval(capture, 200);