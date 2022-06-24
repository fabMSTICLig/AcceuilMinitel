function bytesToString(bytes, offset) {
    var str = "";
    var index = offset;
    while (bytes[index] != 0 && index < bytes.length) {
        str += String.fromCharCode(bytes[index]);
        index++;
    }
    var ret = {}
    ret.str = str
    ret.offset = index + 1
    return ret
}


function pad(number) {
  if ( number < 10 ) {
    return '0' + number;
  }
  return number;
}
function Decoder(bytes, fPort) {
    var obj = new Object();
    obj.cmd = (bytes[0] & 0xF0) >> 4
    obj.subCmd = bytes[0] & 0xF
    if (obj.subCmd == 1) {
        obj.xor = bytes[1];
        obj.len = bytes[2];
      	var date= new Date()
      	obj.date = date.getUTCFullYear() +
        '-' + pad( date.getUTCMonth() + 1 ) +
        '-' + pad( date.getUTCDate() ) +
        ' ' + pad( date.getHours()+1 ) +
        ':' + pad( date.getUTCMinutes() )
        var offset = 3;
        var ret = bytesToString(bytes, offset)
        obj.user = ret.str
        offset = ret.offset
        obj.message = bytesToString(bytes, offset).str
    } else if (obj.subCmd == 2) {
        obj.pos = bytes[1];
        obj.message = bytesToString(bytes, 2).str
    } else if (obj.subCmd == 3) {
        obj.message = bytesToString(bytes, 1).str
    }

    return obj;
}

