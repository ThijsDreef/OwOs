// The MIT License (MIT) Copyright (c) 2015 Matt DesLauriers
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

function parseBMFontAscii(data) {
    if (!data)
      throw new Error('no data provided')
    data = data.toString().trim()
  
    var output = {
      pages: [],
      chars: [],
      kernings: []
    }
  
    var lines = data.split(/\r\n?|\n/g)
  
    if (lines.length === 0)
      throw new Error('no data in BMFont file')
  
    for (var i = 0; i < lines.length; i++) {
      var lineData = splitLine(lines[i], i)
      if (!lineData) //skip empty lines
        continue
  
      if (lineData.key === 'page') {
        if (typeof lineData.data.id !== 'number')
          throw new Error('malformed file at line ' + i + ' -- needs page id=N')
        if (typeof lineData.data.file !== 'string')
          throw new Error('malformed file at line ' + i + ' -- needs page file="path"')
        output.pages[lineData.data.id] = lineData.data.file
      } else if (lineData.key === 'chars' || lineData.key === 'kernings') {
        //... do nothing for these two ...
      } else if (lineData.key === 'char') {
        output.chars.push(lineData.data)
      } else if (lineData.key === 'kerning') {
        output.kernings.push(lineData.data)
      } else {
        output[lineData.key] = lineData.data
      }
    }
  
    return output
  }
  
  function splitLine(line, idx) {
    line = line.replace(/\t+/g, ' ').trim()
    if (!line)
      return null
  
    var space = line.indexOf(' ')
    if (space === -1) 
      throw new Error("no named row at line " + idx)
  
    var key = line.substring(0, space)
  
    line = line.substring(space + 1)
    //clear "letter" field as it is non-standard and
    //requires additional complexity to parse " / = symbols
    line = line.replace(/letter=[\'\"]\S+[\'\"]/gi, '')  
    line = line.split("=")
    line = line.map(function(str) {
      return str.trim().match((/(".*?"|[^"\s]+)+(?=\s*|\s*$)/g))
    })
  
    var data = []
    for (var i = 0; i < line.length; i++) {
      var dt = line[i]
      if (i === 0) {
        data.push({
          key: dt[0],
          data: ""
        })
      } else if (i === line.length - 1) {
        data[data.length - 1].data = parseData(dt[0])
      } else {
        data[data.length - 1].data = parseData(dt[0])
        data.push({
          key: dt[1],
          data: ""
        })
      }
    }
  
    var out = {
      key: key,
      data: {}
    }
  
    data.forEach(function(v) {
      out.data[v.key] = v.data;
    })
  
    return out
  }
  
  function parseData(data) {
    if (!data || data.length === 0)
      return ""
  
    if (data.indexOf('"') === 0 || data.indexOf("'") === 0)
      return data.substring(1, data.length - 1)
    if (data.indexOf(',') !== -1)
      return parseIntList(data)
    return parseInt(data, 10)
  }
  
  function parseIntList(data) {
    return data.split(',').map(function(val) {
      return parseInt(val, 10)
    })
  }