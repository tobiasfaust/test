import boto3

s3 = boto3.client('s3')

get_last_modified = lambda obj: int(obj['LastModified'].strftime('%Y%m%d%H%M%S'))

def delete_old_objects(bucketname, targetpath):
    resp = s3.list_objects(Bucket=bucketname,Prefix=targetpath + "/")
    if 'Contents' in resp:
        objs = resp['Contents']
        files = sorted(objs, key=get_last_modified, reverse=True)
        files_DEV  = []
        files_PRE  = []
        files_PROD = []
        
        for key in files:
            #print(key)
            
            if key['Key'].find(".DEV.") > 0 and len(files_DEV) <= 7 : # 4 Binaries + 4 Json (incl. dem jetzt kommenden)
                files_DEV.append(key)
                print ("Save DEV Object: " + key['Key'])
            elif key['Key'].find(".DEV.") > 0 and len(files_DEV) > 7 :
                print("Delete DEV Object: " + key['Key'])
                s3.delete_object(Bucket=bucketname, Key=key['Key'])
            
            if key['Key'].find(".PRE.") > 0 and len(files_PRE) <= 7 : 
                files_PRE.append(key)
                print ("Save PRE Object: " + key['Key'])
            elif key['Key'].find(".PRE.") > 0 and len(files_PRE) > 7 :
                print("Delete PRE Object: " + key['Key'])
                s3.delete_object(Bucket=bucketname, Key=key['Key'])
                
            if key['Key'].find(".PROD.") > 0 and len(files_PROD) <= 7 : 
                files_PROD.append(key)
                print ("Save PROD Object: " + key['Key'])
            elif key['Key'].find(".PROD.") > 0 and len(files_PROD) > 7 :
                print("Delete PROD Object: " + key['Key'])
                s3.delete_object(Bucket=bucketname, Key=key['Key'])