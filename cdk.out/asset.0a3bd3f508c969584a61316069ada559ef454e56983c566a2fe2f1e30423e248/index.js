"use strict";
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __export = (target, all) => {
  for (var name in all)
    __defProp(target, name, { get: all[name], enumerable: true });
};
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

// lib/iot/code.ota.ts
var code_ota_exports = {};
__export(code_ota_exports, {
  handler: () => handler
});
module.exports = __toCommonJS(code_ota_exports);
var import_aws_sdk = require("aws-sdk");
var iot = new import_aws_sdk.Iot();
var signingProfileName = process.env.signingProfileName;
var thingNamePrefix = process.env.thingNamePrefix;
var destBucketName = process.env.destBucketName;
var roleArn = process.env.roleArn;
async function listThingArns() {
  let things = [];
  let nextToken;
  do {
    const result = await iot.listThings({ maxResults: 1, nextToken }).promise();
    nextToken = result.nextToken;
    things = [...things, ...result.things];
  } while (nextToken);
  return things.filter((thing) => thing.thingName.startsWith(thingNamePrefix)).map((thing) => thing.thingArn);
}
async function handler(event) {
  console.log(JSON.stringify(event));
  const [{ s3 }] = event.Records;
  const { bucket, object } = s3;
  const thingArns = await listThingArns();
  const params = {
    otaUpdateId: Date.now().toString(),
    targets: thingArns,
    targetSelection: "SNAPSHOT",
    files: [
      {
        fileName: object.key.split("/").pop(),
        fileVersion: object.versionId,
        fileLocation: {
          s3Location: {
            bucket: bucket.name,
            key: object.key,
            version: object.versionId
          }
        },
        codeSigning: {
          startSigningJobParameter: {
            signingProfileName,
            destination: {
              s3Destination: {
                bucket: destBucketName
              }
            }
          }
        }
      }
    ],
    roleArn
  };
  const response = await iot.createOTAUpdate(params).promise();
  console.log(JSON.stringify(response));
}
// Annotate the CommonJS export names for ESM import in node:
0 && (module.exports = {
  handler
});
