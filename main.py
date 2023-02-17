import flet as ft
import pandas as pd
import pyrebase

def main(page: ft.Page):
    # secretKey qwertyuiop
    config = {
        "apiKey": "AIzaSyCYLCqr-XaeOfrklNBFfCpgC_MROqFzZyw",
        "authDomain": "dispens-o-tron.firebaseapp.com",
        "databaseURL": "https://dispens-o-tron-default-rtdb.europe-west1.firebasedatabase.app/",
        "projectId": "dispens-o-tron",
        "storageBucket": "dispens-o-tron.appspot.com",
        "messagingSenderId": "1048399231705",
        "appId": "1:1048399231705:web:846ee976b23e7db1089cc8",
    }
    firebase = pyrebase.initialize_app(config)
    db = firebase.database()

    page.title = "Dispens-o-Tron"
    page.vertical_alignment = ft.MainAxisAlignment.CENTER

    emailBox = ft.TextField(hint_text="Student Email", width=300, autofocus=True)
    passBox = ft.TextField(hint_text="Password", width=300)

    # --------------------------------------------------
    # data = {'andy': ['1'], 'john': ['2']}
    # df = pd.DataFrame(data=data)

    # db.child("users").push(d)
    # db.child("users").set(data)

    # --------------------------------------------------

    def route_change(route):
        page.views.clear()
        page.views.append(
            ft.View(
                "/",
                [
                    ft.Row(
                        [
                            emailBox
                        ],
                        alignment=ft.MainAxisAlignment.CENTER,
                    ),
                    ft.Row(
                        [
                            passBox
                        ],
                        alignment=ft.MainAxisAlignment.CENTER,
                    ),
                    ft.Row(
                        [
                            ft.ElevatedButton(text="Submit", on_click=onSubmit)
                        ],
                        alignment=ft.MainAxisAlignment.CENTER,
                    )
                ],
                vertical_alignment=ft.MainAxisAlignment.CENTER
            )
        )
        if page.route == "/success":
            page.vertical_alignment = ft.MainAxisAlignment.CENTER
            page.views.append(
                ft.View(
                    "/success",
                    [
                        ft.Row(
                            [
                                ft.ElevatedButton(text="Success", on_click=lambda _: page.go("/"))
                            ],
                            alignment=ft.MainAxisAlignment.CENTER,
                        )

                    ],
                    vertical_alignment=ft.MainAxisAlignment.CENTER,
                )
            )
        elif page.route == "/fail":
            page.vertical_alignment = ft.MainAxisAlignment.CENTER
            page.views.append(
                ft.View(
                    "/fail",
                    [
                        ft.Row(
                            [
                                ft.ElevatedButton(text="Failed", on_click=lambda _: page.go("/"))
                            ],
                            alignment=ft.MainAxisAlignment.CENTER,
                        )
                    ],
                    vertical_alignment=ft.MainAxisAlignment.CENTER,
                )
            )

        page.update()

    def onSubmit(e):
        email = emailBox.value
        password = passBox.value
        result = validate(email, password)
        if result:
            page.go("/success")
        else:
            page.go("/fail")

    def validate(email, password):
        users = db.child("users").get()
        df = pd.DataFrame(users.val())
        if (email not in df):
            return False
        if (df[email][0]) == password:
            emitSignal()
            return True
        return False

    def view_pop(view):
        page.views.pop()
        top_view = page.views[-1]
        page.go(top_view.route)

    page.on_route_change = route_change
    page.on_view_pop = view_pop
    page.go(page.route)


ft.app(target=main)
