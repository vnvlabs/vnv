function parseLink(entry) {
    try {
        const link = entry.match(/<?([^>]*)>(.*)/),
            url = link[1],
            [, ...parts] = link[2].split(';'),
            query = new URL(url).searchParams

        const parsedLink = parts.reduce((acc, part) => {
            const attr = part.match(/\s*(.+)\s*=\s*"?([^"]+)"?/)
            if (attr) acc[attr[1]] = attr[2]
            return acc
        }, {})

        for (const [key, value] of query) parsedLink[key] = value
        parsedLink.url = url

        return parsedLink
    } catch (e) {
        return null
    }
}

function parseLinkHeader(linkHeader) {
    if (!linkHeader) return null

    return linkHeader.split(/,\s*</)
        .map(parseLink)
        .filter(link => link && link.rel)
        .reduce((acc, link) => {
            link.rel.split(/\s+/).forEach(rel => {
                if (acc[rel] && !Array.isArray(acc[rel])) {
                    acc[rel] = [acc[rel]].concat(Object.assign({}, link))
                    acc[rel][acc[rel].length - 1].rel = rel
                } else if (acc[rel] && Array.isArray(acc[rel])) {
                    acc[rel] = acc[rel].concat(Object.assign({}, link))
                    acc[rel][acc[rel].length - 1].rel = rel
                } else {
                    acc[rel] = Object.assign({}, link)
                    acc[rel].rel = rel
                }
            })
            return acc
        }, {})
}