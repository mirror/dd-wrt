# Hotspot Voucher Generator
#Copyright (C) Francesco Servida 2023

import random
import string

rate_down=1024
rate_up=1024
quota_down=0
quota_up=0
validity=1440 #minutes

voucher_qty=200

def randomstring(length=4):
    letters = string.ascii_uppercase + string.digits
    return "".join(random.choice(letters) for i in range(length))

def new_voucher():
    voucher_code = f"{randomstring()}-{randomstring()}"
    return f"{voucher_code},{rate_down},{rate_up},{quota_down},{quota_up},{validity},0\n"

filepath="vouchers.txt"
with open(filepath, "a") as voucher_file:
    for i in range(voucher_qty):
        voucher_file.write(new_voucher())
