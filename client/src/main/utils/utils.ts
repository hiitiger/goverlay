import * as path from "path"

export function fileUrl(str: string) {
    let pathName = path.resolve(str).replace(/\\/g, "/")

    // Windows drive letter must be prefixed with a slash
    if (pathName[0] !== "/") {
        pathName = "/" + pathName
    }

    return encodeURI("file://" + pathName)
}
