

def add_list(aDict, aKey, aElementToAdd):
    if aKey not in aDict:
        aDict[aKey] = []    
    aDict[aKey].append(aElementToAdd)

def get_dict_or_empty(aDict, aKey):
    if aKey not in aDict:
        return []
    return aDict[aKey]