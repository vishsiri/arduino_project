function doGet(e) {
  Logger.log( JSON.stringify(e) );
  var result = 'Ok';
  if (e.parameter == 'undefined') {
    result = 'No Parameters';
  }
  else {
    var sheet_id = 'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'; 	// Spreadsheet ID
    var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet();
    var newRow = sheet.getLastRow() + 1;
    var rowData = [];
    var Curr_Date = new Date();
    rowData[0] = Curr_Date;
    var Curr_Time = Utilities.formatDate(Curr_Date, "Asia/Bangkok", 'HH:mm:ss');
    rowData[1] = Curr_Time;
    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param);
      var value = stripQuotes(e.parameter[param]);
      Logger.log(param + ':' + e.parameter[param]);
      switch (param) {
        case 'rfid_key':
          rowData[2] = value;
          result = 'rfid_key write col C';
          break;
        case 'std_id':
          rowData[3] = value;
          result = 'student_id write col D';
          break;
        case 'std_name':
          rowData[4] = value;
          result = 'student_name write col E';
          break;
        case 'std_lastname':
          rowData[5] = value;
          result = 'student_lastname write col F';
          break;
        case 'std_blood':
          rowData[6] = value;
          result = 'student_blood write col G';
          break;
      }
    }
    Logger.log(JSON.stringify(rowData));
    var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
    newRange.setValues([rowData]);
  }
  return ContentService.createTextOutput(result);
}
function stripQuotes( value ) {
  return value.replace(/^["']|['"]$/g, "");
}
