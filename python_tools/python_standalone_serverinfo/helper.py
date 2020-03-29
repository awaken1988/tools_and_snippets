
def json_to_html(aJsonData, aHeaderNames):
    content = "<table>"

    content += "<tr>"
    for iHeader in aHeaderNames:
        content += "<th>" + iHeader + "</th>"
    content += "</tr>"

    for iEntry in aJsonData:
        content += "<tr>"
        for iHeader in aHeaderNames:
            val = ""
            if iHeader in iEntry: val = str(iEntry[iHeader])
            if not val: val = ""
            content += "<td>" + val + "</td>"
        content += "</tr>"

    content += "</table>"

    return content


