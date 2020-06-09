type t =
  | Raw(Js.Typed_array.array_buffer)
  | Text(string)
  | Json(Js.Json.t)
  | Form(formT)
  | NoBody
and formT = list((string, string));

module Buffer = {
  type t;
  [@bs.scope "Buffer"] [@bs.val] external from: 'a => 'b = "from";
  [@bs.scope "Buffer"] [@bs.val] external concat: (('a, 'b)) => 'c = "from";
  [@bs.send.pipe: 'a] external toString: string => string = "toString";
  [@bs.send.pipe: 'a] external toJSON: unit => Js.Json.t = "toJSON";
};

// TODO: rewrite this to reason
let get:
  (Js.Typed_array.array_buffer => unit, unit => unit, Uws_Response.t) => unit = [%bs.raw
  {|
      function(cb, err, res) {
        let buffer

        res.onData((bytes, isLast) => {
          const chunk = Buffer.from(bytes)
          if (isLast) {
            if (!buffer) {
              buffer = chunk
            };

            cb(buffer);
          } else {
            if (buffer) {
              buffer = Buffer.concat([buffer, chunk])
            } else {
              buffer = chunk
            }
          }
        })

        res.onAborted(err)
      }
  |}
];

let parse = (body, contentType) => {
  switch (contentType) {
  | "application/json" =>
    try(Json(body |> Buffer.toString("utf-8") |> Js.Json.parseExn)) {
    | _ => Raw(body)
    }
  | "application/x-www-form-urlencoded" =>
    Json(body |> Buffer.toString("utf-8") |> Qs.parse)
  | "text/plain" => Text(body |> Buffer.toString("utf-8"))
  | _ => Raw(body)
  };
};